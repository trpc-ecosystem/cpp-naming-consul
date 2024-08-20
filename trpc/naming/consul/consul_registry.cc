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

#include "trpc/naming/consul/consul_registry.h"

#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/document.h"

#include "trpc/common/config/trpc_config.h"
#include "trpc/naming/consul/config/consul_naming_conf.h"
#include "trpc/naming/consul/consul.h"

namespace trpc {

int ConsulRegistry::Init() noexcept {
  trpc::naming::ConsulConfig config;
  if (!trpc::TrpcConfig::GetInstance()->GetPluginConfig<trpc::naming::ConsulConfig>(
          "registry", "consul", config)) {
    TRPC_FMT_ERROR("get selector consul config error, use default value");
    return -1;
  }
  config.Display();

  consul_config_ = std::any_cast<trpc::naming::ConsulConfig>(config);

  int ret = curl_http_.Init();
  if (ret != trpc::curl_http::kOk) {
    return -1;
  }
  init_ = true;
  return 0;
}

void ConsulRegistry::Destroy() noexcept {
  if (!init_) {
    TRPC_FMT_DEBUG("No init yet");
    return;
  }

  init_ = false;
}

int ConsulRegistry::Register(const trpc::RegistryInfo* info) {
  if (info == nullptr) {
    TRPC_FMT_ERROR("registryInfo is null");
    return -1;
  }
  std::string registerPath = "http://" + consul_config_.address_ + "/v1/agent/service/register";
  std::string body = ConstructRegisterJson(info);
  trpc::curl_http::CurlHttpResponsePtr response = curl_http_.Put(registerPath, body);
  if (response->response_code != trpc::curl_http::kHttpStatusCode200) {
    TRPC_FMT_ERROR("register service err ret code{}", response->response_code);
    return -1;
  }
  return 0;
}

int ConsulRegistry::Unregister(const trpc::RegistryInfo* info) {
  std::string deregister_path = "http://" + consul_config_.address_ + "/v1/agent/service/deregister/" + info->name;
  trpc::curl_http::CurlHttpResponsePtr response = curl_http_.Put(deregister_path, "");
  if (response->response_code != trpc::curl_http::kHttpStatusCode200) {
    TRPC_FMT_ERROR("unregister service err ret code{}", response->response_code);
    return -1;
  }
  return 0;
}

std::string ConsulRegistry::ConstructRegisterJson(const trpc::RegistryInfo* info) const {
  rapidjson::Document d;
  d.SetObject();

  d.AddMember(::rapidjson::StringRef("name"), ::rapidjson::StringRef(info->name.c_str()), d.GetAllocator());
  d.AddMember(::rapidjson::StringRef("Address"), ::rapidjson::StringRef(info->host.c_str()), d.GetAllocator());
  d.AddMember(::rapidjson::StringRef("Port"), info->port, d.GetAllocator());

  rapidjson::Value meta(rapidjson::kObjectType);
  for (auto& metaItem : info->meta) {
    meta.AddMember(::rapidjson::StringRef(metaItem.first.c_str()),
                   ::rapidjson::StringRef(metaItem.second.c_str()),
                   d.GetAllocator());
  }
  d.AddMember(::rapidjson::StringRef("meta"), meta, d.GetAllocator());

  ::rapidjson::StringBuffer buffer;
  ::rapidjson::Writer<::rapidjson::StringBuffer> writer(buffer);
  d.Accept(writer);
  return buffer.GetString();
}

}  // namespace trpc
