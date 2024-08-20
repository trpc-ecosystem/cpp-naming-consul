//
//
// Tencent is pleased to support the open source community by making tRPC available.
//
// Copyright (C) 2024 THL A29 Limited, a Tencent company.
// All rights reserved.
//
// If you have downloaded a copy of the tRPC source code from Tencent,
// please note that tRPC source code is licensed under the  Apache 2.0 License,
// A copy of the Apache 2.0 License is included in this file.
//
//

#include "trpc/naming/consul/consul_selector.h"

#include <map>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "google/protobuf/util/json_util.h"

#include "trpc/codec/trpc/trpc.pb.h"
#include "trpc/common/config/trpc_config.h"
#include "trpc/naming/selector_factory.h"
#include "trpc/naming/selector.h"
#include "trpc/naming/load_balance_factory.h"
#include "trpc/naming/common/util/loadbalance/polling/polling_load_balance.h"
#include "trpc/naming/consul/config/consul_naming_conf.h"
#include "trpc/runtime/common/periphery_task_scheduler.h"
#include "trpc/util/time.h"
#include "trpc/util/log/logging.h"
#include "trpc/util/string_helper.h"

namespace trpc {

int ConsulSelector::Init() noexcept {
  // Update the cache every 10 seconds
  dn_update_interval_ = 10 * 1000;

  task_id_ = 0;

  trpc::naming::ConsulConfig config;
  if (!trpc::TrpcConfig::GetInstance()->GetPluginConfig<trpc::naming::ConsulConfig>(
        "selector", "consul", config)) {
    TRPC_FMT_INFO("get selector consul config error, use default value");
    return -1;
  }
  config.Display();
  consul_config_ = std::any_cast<trpc::naming::ConsulConfig>(config);

  default_load_balance_ = MakeRefCounted<PollingLoadBalance>();

  return curl_http_.Init();
}

void ConsulSelector::Destroy() noexcept {
    curl_http_.Destroy();
    return;
}

int ConsulSelector::Select(const SelectorInfo* info, TrpcEndpointInfo* endpoint) {
  if (nullptr == info || nullptr == endpoint) {
    TRPC_LOG_ERROR("Invalid parameter");
    return -1;
  }

  if (!InitEndpointInfo(info)) {
    return -1;
  }

  LoadBalanceResult load_balance_result;
  load_balance_result.info = info;
  auto lb = GetLoadBalance(info->load_balance_name);
  if (lb == nullptr) {
    TRPC_LOG_ERROR("get loadbalance err");
    return -1;
  }
  if (lb->Next(load_balance_result)) {
    TRPC_LOG_ERROR("Do load balance of " << info->name << " failed");
    return -1;
  }
  *endpoint = std::any_cast<TrpcEndpointInfo>(load_balance_result.result);
  return 0;
}

Future<TrpcEndpointInfo> ConsulSelector::AsyncSelect(const SelectorInfo* info) {
  if (info == nullptr) {
    TRPC_LOG_ERROR("Selector info is null");
    return MakeExceptionFuture<TrpcEndpointInfo>(CommonException("Selector info is null"));
  }
  TrpcEndpointInfo endpoint;
  int ret = Select(info, &endpoint);
  if (ret != 0) {
    return MakeExceptionFuture<TrpcEndpointInfo>(CommonException("AsyncSelect error"));
  }
  return MakeReadyFuture<TrpcEndpointInfo>(std::move(endpoint));
}

int ConsulSelector::SelectBatch(const SelectorInfo* info, std::vector<TrpcEndpointInfo>* endpoints) {
    if (nullptr == info || nullptr == endpoints) {
    TRPC_LOG_ERROR("Invalid parameter");
    return -1;
  }

  if (!InitEndpointInfo(info)) {
    return -1;
  }

  std::string callee = info->name;
  std::shared_lock<std::shared_mutex> lock(mutex_);
  auto iter = targets_map_.find(callee);
  if (iter == targets_map_.end()) {
    TRPC_LOG_ERROR("router info of " << callee << " no found");
    return -1;
  }
  if (info->policy == SelectorPolicy::MULTIPLE) {
    SelectMultiple(iter->second.endpoints, endpoints, info->select_num);
  } else {
    *endpoints = iter->second.endpoints;
  }
  return 0;
}

Future<std::vector<TrpcEndpointInfo>> ConsulSelector::AsyncSelectBatch(const SelectorInfo* info) {
  if (info == nullptr) {
    TRPC_LOG_ERROR("Invalid parameter");
    return MakeExceptionFuture<std::vector<TrpcEndpointInfo>>(CommonException("Invalid SelectorInfo"));
  }
  std::vector<TrpcEndpointInfo> endpoints;
  int ret = SelectBatch(info, &endpoints);
  if (ret != 0) {
    return MakeExceptionFuture<std::vector<TrpcEndpointInfo>>(CommonException("AsyncSelectBatch error"));
  }
  return MakeReadyFuture<std::vector<TrpcEndpointInfo>>(std::move(endpoints));
}

int ConsulSelector::ReportInvokeResult(const InvokeResult* result) {
    if (nullptr == result) {
    TRPC_LOG_ERROR("Invalid parameter: invoke result is empty");
    return -1;
  }
  return 0;
}

bool ConsulSelector::ParseResponse(const rapidjson::Document& resp_body, const std::string& service_name,
                                   std::vector<TrpcEndpointInfo>& endpoints) {
  const auto& nodes = resp_body.GetArray();
  for (auto it = nodes.Begin(); it != nodes.End(); ++it) {
    auto node = it->GetObject();
    if (!node.HasMember("Service") || !node["Service"].IsObject()) {
      TRPC_LOG_ERROR("Service not exist or is not object");
      return false;
    }
    if (!node["Service"].HasMember("Address") || !node["Service"]["Address"].IsString() ||
        !node["Service"].HasMember("Port") || !node["Service"]["Port"].IsInt()) {
          TRPC_LOG_ERROR("Address or Port fmt err");
          return false;
    }
    // Get IP:Port
    TrpcEndpointInfo endpoint;
    endpoint.host = node["Service"]["Address"].GetString();
    endpoint.port = node["Service"]["Port"].GetInt();
    endpoint.is_ipv6 = (endpoint.host.find(':') != std::string::npos);
    // Get health status
    if (!node.HasMember("Checks") || !node["Checks"].IsArray()) {
      TRPC_LOG_ERROR("Checks not exist or is not Array");
      return false;
    }
    const auto& checks = node["Checks"].GetArray();
    for (auto it = checks.Begin(); it != checks.End(); ++it) {
        auto check = it->GetObject();
        if (!check.HasMember("Name") || !check["Name"].IsString() ||
           !check.HasMember("Status") || !check["Status"].IsString()) {
          continue;
        }
        std::string checkName = check["Name"].GetString();
        if (checkName != service_name) {  // Skip the check information of the agent itself.
          continue;
        }
        const std::string& status = check["Status"].GetString();
        // If the node's health status is abnormal, assign a value of -1 to the status.
        endpoint.status = (status == "passing") ? 0 : -1;
    }

    TRPC_LOG_DEBUG("host:" << endpoint.host << ",port:" << endpoint.port);
    TRPC_LOG_DEBUG("is_ipv6:" << endpoint.is_ipv6 << ",status:" << endpoint.status);
    endpoints.emplace_back(endpoint);
  }

  if (endpoints.size() > 0) {
    return true;
  }

  TRPC_LOG_ERROR("Response body contains no endpoints");
  return false;
}

int ConsulSelector::RefreshEndpointInfoByName(const SelectorInfo* info, DomainEndpointInfo& endpointInfo) {
  std::string deregister_path = "http://" + consul_config_.address_ + "/v1/health/service/" + info->name;
  trpc::curl_http::CurlHttpResponsePtr response = curl_http_.Get(deregister_path);
  if (response->response_code != trpc::curl_http::kHttpStatusCode200) {
    TRPC_LOG_ERROR("consul resp errcode:" << response->response_code);
    return -1;
  }
  rapidjson::Document resp_body;
  if (resp_body.Parse(response->body.c_str()).HasParseError()) {
    TRPC_LOG_ERROR("parse response body err");
    return -1;
  }
  if (!resp_body.IsArray()) {
    TRPC_LOG_ERROR("resp body is not array");
    return -1;
  }

  std::vector<TrpcEndpointInfo> endpoints;
  if (ParseResponse(resp_body, info->name, endpoints)) {
    endpointInfo.domain_name = info->name;
    endpointInfo.endpoints.swap(endpoints);
    return 0;
  }

  return -1;
}

int ConsulSelector::RefreshDomainInfo(const SelectorInfo* info, ConsulSelector::DomainEndpointInfo& dn_endpointInfo) {
  if (nullptr == info) {
    TRPC_LOG_ERROR("Invalid parameter");
    return -1;
  }
  std::unique_lock<std::shared_mutex> uniq_lock(mutex_);
  auto iter = targets_map_.find(info->name);
  if (iter != targets_map_.end()) {
    dn_endpointInfo.id_generator = std::move(iter->second.id_generator);
  }
  for (auto& item : dn_endpointInfo.endpoints) {
    std::string endpoint = item.host + ":" + std::to_string(item.port);
    item.id = dn_endpointInfo.id_generator.GetEndpointId(endpoint);
  }
  targets_map_[info->name] = dn_endpointInfo;
  uniq_lock.unlock();
  // update loadbalance cache
  LoadBalanceInfo lb_info;
  lb_info.info = info;
  lb_info.endpoints = &dn_endpointInfo.endpoints;
  default_load_balance_->Update(&lb_info);
  return 0;
}

bool ConsulSelector::InitEndpointInfo(const SelectorInfo* info) {
  // If this service is selected first time, and it does not exist in the cache, it needs to be retrieved from Consul.
  bool queryed = false;
  std::unique_lock<std::shared_mutex> uniq_lock(mutex_);
  auto iter = targets_map_.find(info->name);
  if (iter != targets_map_.end()) {
    queryed = true;
  }
  uniq_lock.unlock();
  if (!queryed) {
    ConsulSelector::DomainEndpointInfo endpointInfo;
    if (!RefreshEndpointInfoByName(info, endpointInfo)) {
      int ret = RefreshDomainInfo(info, endpointInfo);
      if (ret != 0) {
        TRPC_LOG_ERROR("refresh domain info err:" << ret);
        return false;
      }
    }
  }

  return true;
}

bool ConsulSelector::NeedUpdate() {
  uint64_t current_time = trpc::time::GetMilliSeconds();
  uint64_t next_refresh_time = last_update_time_ + dn_update_interval_;
  if (next_refresh_time < current_time) {
    last_update_time_ = current_time;
    return true;
  }
  return false;
}

int ConsulSelector::UpdateEndpointInfo() {
  std::shared_lock<std::shared_mutex> lock(mutex_);
  auto targets_map = targets_map_;  // copy-modify-update
  lock.unlock();
  int targets_count = targets_map_.size();
  int success_count = 0;

  for (auto item : targets_map) {
    ConsulSelector::DomainEndpointInfo endpointInfo;
    SelectorInfo selectorInfo;
    selectorInfo.name = item.second.domain_name;
    if (!RefreshEndpointInfoByName(&selectorInfo, endpointInfo)) {
      TRPC_LOG_DEBUG("Update endpointInfo of " << item.first << ":" << item.second.domain_name << " success");
      SelectorInfo selector_info;
      selector_info.name = item.first;
      RefreshDomainInfo(&selector_info, endpointInfo);
      success_count++;
    }
  }
  return (targets_count == 0 || success_count > 0) ? 0 : -1;
}

LoadBalance* ConsulSelector::GetLoadBalance(const std::string& name) {
  if (!name.empty()) {
    auto load_balance = LoadBalanceFactory::GetInstance()->Get(name).get();
    if (load_balance) {
      return load_balance;
    }
  }

  return default_load_balance_.get();
}

int ConsulSelector::SetEndpoints(const RouterInfo* info) {
  if (info == nullptr) {
    TRPC_LOG_ERROR("Invalid parameter: router info is empty");
    return -1;
  }
  std::string callee_name = info->name;
  if (info->info.size() != 1) {
    TRPC_LOG_ERROR("Router info is invalid");
    return -1;
  }
  std::string dn_name = info->info[0].host;
  SelectorInfo selector_info;
  selector_info.name = info->info[0].host;
  ConsulSelector::DomainEndpointInfo endpointInfo;
  if (0 != RefreshEndpointInfoByName(&selector_info, endpointInfo)) {
    TRPC_LOG_ERROR("RefreshEndpointInfoByName of name" << dn_name << " failed");
    return -1;
  }
  RefreshDomainInfo(&selector_info, endpointInfo);
  return 0;
}

void ConsulSelector::Start() noexcept {
  TRPC_LOG_DEBUG("Start consul selector task");
  if (task_id_ == 0) {
    task_id_ = PeripheryTaskScheduler::GetInstance()->SubmitInnerPeriodicalTask(
        [this]() {
          if (NeedUpdate()) {
            UpdateEndpointInfo();
          }
          TRPC_LOG_TRACE("SelectorDomainTask Running");
        },
        200, "ConsulSelector");
  }
}

void ConsulSelector::Stop() noexcept {
  if (task_id_) {
    PeripheryTaskScheduler::GetInstance()->StopInnerTask(task_id_);
    task_id_ = 0;
  }
}

}  // namespace trpc
