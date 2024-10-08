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

#include "trpc/naming/consul/consul_selector_api.h"

#include "trpc/common/trpc_plugin.h"
#include "trpc/naming/consul/consul_selector.h"
#include "trpc/naming/consul/consul_selector_filter.h"

namespace trpc::consul::selector {

bool Init() {
  TrpcPlugin::GetInstance()->RegisterSelector(MakeRefCounted<ConsulSelector>());
  TrpcPlugin::GetInstance()->RegisterClientFilter(std::make_shared<ConsulSelectorFilter>());

  return true;
}

}  // namespace trpc::consul::selector
