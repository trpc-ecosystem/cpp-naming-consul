licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "client",
    srcs = ["client.cc"],
    deps = [
        "//examples/server:helloworld_proto",
        #"//trpc/naming/consul:consul_limiter_api",
        "//trpc/naming/consul:consul_selector_api",
        "@com_github_gflags_gflags//:gflags",
        "@trpc_cpp//trpc/client:client_context",
        "@trpc_cpp//trpc/client:make_client_context",
        "@trpc_cpp//trpc/client:trpc_client",
        "@trpc_cpp//trpc/common:runtime_manager",
        "@trpc_cpp//trpc/common/config:trpc_config",
    ],
)
