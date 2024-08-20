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

#include <memory>
#include <string>
#include <vector>

#include "trpc/filter/filter.h"
#include "trpc/naming/selector_workflow.h"

namespace trpc {

class ConsulSelectorFilter : public MessageClientFilter {
 public:
  ConsulSelectorFilter() {
    selector_flow_ = std::make_unique<SelectorWorkFlow>("consul", true, true);
  }

  ~ConsulSelectorFilter() override {}

  int Init() override { return selector_flow_->Init(); }

  std::string Name() override { return "consul"; }

  FilterType Type() override { return FilterType::kSelector; }

  std::vector<FilterPoint> GetFilterPoint() override {
    std::vector<FilterPoint> points = {FilterPoint::CLIENT_PRE_RPC_INVOKE,
                                       FilterPoint::CLIENT_POST_RPC_INVOKE};
    return points;
  }

  void operator()(FilterStatus& status, FilterPoint point,
                  const ClientContextPtr& context) override {
    selector_flow_->RunFilter(status, point, context);
  }

 private:
  std::unique_ptr<SelectorWorkFlow> selector_flow_;
};

}  // namespace trpc

