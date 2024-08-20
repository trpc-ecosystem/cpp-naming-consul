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

#include <memory>

#include "gtest/gtest.h"

#include "trpc/common/config/trpc_config.h"
#include "trpc/future/future_utility.h"
#include "trpc/runtime/common/periphery_task_scheduler.h"

namespace trpc {

constexpr char kServiceName[] = "testconfig";
constexpr char kHostIp[] = "127.0.0.1";
constexpr uint16_t kHostPort = 80;

TEST(ConsulSelectorTest, timetasktest) {
  auto ret = trpc::TrpcConfig::GetInstance()->Init("./trpc/naming/consul/testing/consul_test.yaml");
  EXPECT_TRUE(ret == 0);
  PeripheryTaskScheduler::GetInstance()->Init();
  PeripheryTaskScheduler::GetInstance()->Start();

  std::shared_ptr<ConsulSelector> ptr = std::make_shared<ConsulSelector>();
  std::any params;
  ptr->Init();
  ptr->Start();

  EXPECT_TRUE(ptr->Version() != "");

  // set endpoints of callee
  RouterInfo info;
  info.name = kServiceName;
  std::vector<TrpcEndpointInfo>& endpoints_info = info.info;
  TrpcEndpointInfo endpoint1;
  endpoint1.host = kServiceName;
  endpoint1.port = 1001;
  endpoints_info.push_back(endpoint1);
  ret = ptr->SetEndpoints(&info);
  EXPECT_EQ(ret, 0);
}

TEST(ConsulSelectorTest, select_test) {
  auto ret = trpc::TrpcConfig::GetInstance()->Init("./trpc/naming/consul/testing/consul_test.yaml");
  EXPECT_TRUE(ret == 0);
  std::shared_ptr<ConsulSelector> ptr = std::make_shared<ConsulSelector>();
  std::any params;
  ptr->Init();

  auto context = trpc::MakeRefCounted<trpc::ClientContext>();
  SelectorInfo select_info;
  select_info.name = kServiceName;
  select_info.context = context;
  TrpcEndpointInfo endpoint;

  ptr->Select(&select_info, &endpoint);
  if (endpoint.is_ipv6) {
    EXPECT_TRUE(endpoint.host == "::1");
  } else {
    EXPECT_TRUE(endpoint.host == kHostIp);
  }
  EXPECT_TRUE(endpoint.port == kHostPort);
  EXPECT_TRUE(endpoint.id != kInvalidEndpointId);

  auto fut = ptr->AsyncSelect(&select_info).Then([](Future<trpc::TrpcEndpointInfo>&& fut) {
    TrpcEndpointInfo endpoint = fut.GetValue0();
    if (endpoint.is_ipv6) {
      EXPECT_TRUE(endpoint.host == "::1");
    } else {
      EXPECT_TRUE(endpoint.host == kHostIp);
    }
    EXPECT_TRUE(endpoint.port == kHostPort);
    EXPECT_TRUE(endpoint.id != kInvalidEndpointId);
    return MakeReadyFuture<>();
  });

  trpc::future::BlockingGet(std::move(fut));
}

TEST(ConsulSelectorTest, invock_report_result_test) {
  auto ret = trpc::TrpcConfig::GetInstance()->Init("./trpc/naming/consul/testing/consul_test.yaml");
  EXPECT_TRUE(ret == 0);
  std::shared_ptr<ConsulSelector> ptr = std::make_shared<ConsulSelector>();
  std::any params;
  ptr->Init();

  InvokeResult result;
  result.framework_result = 0;
  result.interface_result = 0;
  result.cost_time = 100;
  result.context = MakeRefCounted<ClientContext>();
  int report_ret = ptr->ReportInvokeResult(&result);
  EXPECT_EQ(0, report_ret);

  EXPECT_TRUE(ptr->ReportInvokeResult(nullptr) != 0);
}


TEST(ConsulSelectorTest, select_batch_test) {
  auto ret = trpc::TrpcConfig::GetInstance()->Init("./trpc/naming/consul/testing/consul_test.yaml");
  EXPECT_TRUE(ret == 0);
  std::shared_ptr<ConsulSelector> ptr = std::make_shared<ConsulSelector>();
  std::any params;
  ptr->Init();

  std::vector<TrpcEndpointInfo> endpoints;
  auto context = trpc::MakeRefCounted<trpc::ClientContext>();
  SelectorInfo select_info;
  select_info.name = kServiceName;
  select_info.context = context;
  ptr->SelectBatch(&select_info, &endpoints);
  for (const auto& ref : endpoints) {
    if (ref.is_ipv6) {
      EXPECT_TRUE(ref.host == "::1");
    } else {
      EXPECT_TRUE(ref.host == kHostIp);
    }
    EXPECT_TRUE(ref.port == kHostPort);
    EXPECT_TRUE(ref.id != kInvalidEndpointId);
  }

  endpoints.clear();
  select_info.policy = SelectorPolicy::MULTIPLE;
  ret = ptr->SelectBatch(&select_info, &endpoints);
  EXPECT_EQ(0, ret);
  EXPECT_TRUE(endpoints.size() > 0);

  // negtive case
  select_info.name = "baidu1";
  ret = ptr->SelectBatch(&select_info, &endpoints);
  EXPECT_NE(0, ret);

  ptr->Stop();
  ptr->Destroy();
}

}  // namespace trpc
