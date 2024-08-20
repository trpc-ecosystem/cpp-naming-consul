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

#include <memory>

#include "gtest/gtest.h"

#include "trpc/runtime/common/periphery_task_scheduler.h"
#include "trpc/common/config/trpc_config.h"

namespace trpc {

TEST(ConsulRegistryTest, register_test) {
  auto ret = trpc::TrpcConfig::GetInstance()->Init("./trpc/naming/consul/testing/consul_test.yaml");
  EXPECT_TRUE(ret == 0);

  std::shared_ptr<ConsulRegistry> ptr = std::make_shared<ConsulRegistry>();
  ptr->Init();
  ptr->Start();

  std::string callee_service = "testconfig";
  trpc::RegistryInfo register_info;
  register_info.host = "127.0.0.1";
  register_info.port = 10001;

  // error when input parameter is nullptr
  trpc::RegistryInfo* register_info_no_exist = nullptr;
  ret = ptr->Register(register_info_no_exist);
  EXPECT_EQ(-1, ret);

  // service no exist in register config
  register_info.name = "test.service.no12";
  ret = ptr->Register(&register_info);
  EXPECT_EQ(0, ret);
  ret = ptr->Unregister(&register_info);
  EXPECT_EQ(0, ret);

  // service exists in the register config
  register_info.name = callee_service;
  ret = ptr->Register(&register_info);
  ASSERT_EQ(0, ret);
}

}  // namespace trpc
