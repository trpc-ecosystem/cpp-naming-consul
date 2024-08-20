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

#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include "trpc/naming/consul/consul.h"
#include "trpc/naming/consul/config/consul_naming_conf.h"
#include "trpc/naming/registry.h"
#include "trpc/transport/common/http/curl_http.h"

namespace trpc {

// Consul service registry plugin
class ConsulRegistry : public Registry {
 public:
  std::string Name() const override { return kConsulPluginName; }

  std::string Version() const { return kConsulSDKVersion; }

  int Init() noexcept override;

  void Start() noexcept override {}

  void Stop() noexcept override {}

  void Destroy() noexcept override;

  int Register(const RegistryInfo* info) override;

  int Unregister(const RegistryInfo* info) override;

  int HeartBeat(const RegistryInfo* info) override { return 0; }

  Future<> AsyncHeartBeat(const RegistryInfo* info) override {return MakeReadyFuture<>();}

 private:
  int HealthRegister(const std::string& service_name, const std::string& health_url, const std::string& check_interval);

  std::string ConstructRegisterJson(const trpc::RegistryInfo* info) const;

 private:
  bool init_{false};
  uint64_t heartbeat_interval_;
  uint64_t heartbeat_timeout_;
  trpc::curl_http::CurlHttp curl_http_;

  trpc::naming::ConsulConfig consul_config_;
};


}  // namespace trpc
