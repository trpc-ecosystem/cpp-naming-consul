# Description: trpc-cpp.

load("@trpc_cpp//trpc:trpc.bzl", "trpc_proto_library")

licenses(["notice"])

package(default_visibility = ["//visibility:public"])

trpc_proto_library(
    name = "helloworld_proto",
    srcs = ["helloworld.proto"],
    generate_mock_code = True,
    rootpath = "@trpc_cpp",
    use_trpc_plugin = True,
)

cc_binary(
    name = "helloworld_server",
    srcs = ["helloworld_server.cc"],
    deps = [
        ":greeter_service",
        ":helloworld_proto",
        "@com_github_fmtlib_fmt//:fmtlib",
        "@trpc_cpp//trpc/common:trpc_app",
        #"//trpc/naming/consul:consul_limiter_api",
        "//trpc/naming/consul:consul_selector_api",
        "//trpc/naming/consul:consul_registry_api",
    ],
)

cc_library(
    name = "greeter_service",
    srcs = ["greeter_service.cc"],
    hdrs = ["greeter_service.h"],
    deps = [
        ":helloworld_proto",
        "@trpc_cpp//trpc/log:trpc_log",
    ],
)
