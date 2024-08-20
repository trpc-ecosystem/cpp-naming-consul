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

#pragma once

#include <any>
#include <memory>
#include <set>
#include <string>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include "trpc/naming/common/util/utils_help.h"
#include "trpc/naming/consul/consul.h"
#include "trpc/naming/consul/config/consul_naming_conf.h"
#include "trpc/naming/load_balance.h"
#include "trpc/naming/selector.h"
#include "trpc/transport/common/http/curl_http.h"

namespace trpc {

/// @brief Consul service discovery plugin
class ConsulSelector : public Selector {
 public:
  std::string Name() const override { return kConsulPluginName; }

  std::string Version() const override { return kConsulSDKVersion; }

  int Init() noexcept override;

  void Start() noexcept override;

  void Stop() noexcept override;

  void Destroy() noexcept override;

  int Select(const SelectorInfo* info, TrpcEndpointInfo* endpoint) override;

  Future<TrpcEndpointInfo> AsyncSelect(const SelectorInfo* info) override;

  int SelectBatch(const SelectorInfo* info, std::vector<TrpcEndpointInfo>* endpoints) override;

  Future<std::vector<TrpcEndpointInfo>> AsyncSelectBatch(const SelectorInfo* info) override;

  int ReportInvokeResult(const InvokeResult* result) override;

  int SetEndpoints(const RouterInfo* info) override;

 private:
  bool InitEndpointInfo(const SelectorInfo* info);

  bool NeedUpdate();

  int UpdateEndpointInfo();

  struct DomainEndpointInfo {
    // Domain name of the called service
    std::string domain_name;
    // Port of the called service
    int port;
    // Endpoint info of the called service
    std::vector<TrpcEndpointInfo> endpoints;
    // id generator for endpoint
    EndpointIdGenerator id_generator;
  };

  bool ParseResponse(const rapidjson::Document& resp_body, const std::string& service_name,
                     std::vector<TrpcEndpointInfo>& endpoints);

  int RefreshEndpointInfoByName(const SelectorInfo *info, DomainEndpointInfo& dn_endpointInfo);

  int RefreshDomainInfo(const SelectorInfo *info, DomainEndpointInfo& dn_endpointInfo);

  LoadBalance* GetLoadBalance(const std::string& name);

  bool init_{false};

  LoadBalancePtr default_load_balance_;

  uint64_t timeout_;
  curl_http::CurlHttp curl_http_;

  naming::ConsulConfig consul_config_;

  uint64_t last_update_time_;

  int dn_update_interval_;

  uint64_t task_id_{0};

  std::unordered_map<std::string, DomainEndpointInfo> targets_map_;
  mutable std::shared_mutex mutex_;  // mutex for targets_map_
};

}  // namespace trpc
