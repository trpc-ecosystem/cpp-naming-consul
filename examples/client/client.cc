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

#include "gflags/gflags.h"

#include "trpc/client/client_context.h"
#include "trpc/client/make_client_context.h"
#include "trpc/client/trpc_client.h"
#include "trpc/common/logging/trpc_logging.h"
#include "trpc/common/runtime_manager.h"
#include "trpc/common/status.h"
#include "trpc/naming/consul/consul_selector_api.h"

#include "examples/server/helloworld.trpc.pb.h"

DEFINE_string(config, "examples/naming/consul/client/trpc_cpp_fiber.yaml", "yaml");
DEFINE_string(target, "trpc.test.helloworld.Greeter", "callee service name");
DEFINE_uint32(fibernum, 8, "");
DEFINE_uint32(count, 100, "");
DEFINE_string(msg, "test", "the msg to send");

void ParseClientConfig(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::CommandLineFlagInfo info;
  if (GetCommandLineFlagInfo("config", &info) && info.is_default) {
    std::cerr << "start client with config, for example: " << argv[0] << " --config=/client/config/filepath"
              << std::endl;
    exit(-1);
  }

  int ret = ::trpc::TrpcConfig::GetInstance()->Init(FLAGS_config);
  if (ret != 0) {
    std::cerr << "load config failed." << std::endl;
    exit(-1);
  }
}

void DoRoute(const std::shared_ptr<::trpc::test::helloworld::GreeterServiceProxy>& prx) {
  ::trpc::test::helloworld::HelloRequest request;
  request.set_msg(FLAGS_msg);

  auto context = ::trpc::MakeClientContext(prx);
  context->SetTimeout(1000);

  // If the service level parameter is not set before, you can set the request level parameter in context
  // ::trpc::naming::consul::SetSelectorExtendInfo(context, std::make_pair("namespace", "Development"));

  ::trpc::test::helloworld::HelloReply reply;
  ::trpc::Status status = prx->SayHello(context, request, &reply);
  if (status.OK()) {
    std::cout << "SayHello OK, reply = " << reply.msg() << std::endl;
  } else {
    std::cout << "SayHello failed, error = " << status.ErrorMessage() << std::endl;
  }
}

int Run() {
  ::trpc::ServiceProxyOption option;

  option.name = FLAGS_target;
  option.codec_name = "trpc";
  option.network = "tcp";
  option.conn_type = "long";
  option.timeout = 1000;
  option.selector_name = "consul";
  option.target = FLAGS_target;
  // If you want to set the naming service level parameter, you can do so in the option object
  std::any extend_select_info(std::unordered_map<std::string, std::string>{{"namespace", "Development"}});
  option.service_filter_configs["consul"] = extend_select_info;

  auto prx = ::trpc::GetTrpcClient()->GetProxy<::trpc::test::helloworld::GreeterServiceProxy>(FLAGS_target, &option);

  DoRoute(prx);

  return 0;
}

int main(int argc, char* argv[]) {
  ParseClientConfig(argc, argv);

  // Initializes the consul plugin and filters after start runtime
  ::trpc::consul::selector::Init();

  // If the business code is running in trpc pure client mode,
  // the business code needs to be running in the `RunInTrpcRuntime` function
  return ::trpc::RunInTrpcRuntime([]() { return Run(); });
}
