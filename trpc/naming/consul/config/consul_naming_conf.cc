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

#include "trpc/naming/consul/config/consul_naming_conf.h"
#include "trpc/util/log/logging.h"

namespace trpc::naming {

void ConsulConfig::Display() const {
  TRPC_LOG_DEBUG("--------------------------------");

  TRPC_LOG_DEBUG("address:" << address_);

  TRPC_LOG_DEBUG("--------------------------------");
}

}  // namespace trpc::naming
