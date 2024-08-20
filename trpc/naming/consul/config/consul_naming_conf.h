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
#include <string>
#include <vector>
#include <iostream>

#include "yaml-cpp/yaml.h"


namespace trpc::naming {

struct ConsulConfig {
  std::string address_;

  void Display() const;
};

}  // namespace trpc::naming

namespace YAML {

template <>
struct convert<trpc::naming::ConsulConfig> {
  static YAML::Node encode(const trpc::naming::ConsulConfig& config) {
    YAML::Node node;

    node["address"] = config.address_;

    return node;
  }

  static bool decode(const YAML::Node& node, trpc::naming::ConsulConfig& config) {
    if (node["address"]) {
      config.address_ = node["address"].as<std::string>();
    }

    return true;
  }
};

}  // namespace YAML