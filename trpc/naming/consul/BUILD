# Description: trpc-cpp.

licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "consul_selector",
    srcs = ["consul_selector.cc"],
    hdrs = [
        "consul.h",
        "consul_selector.h"
    ],
    visibility = [
        "//visibility:public",
    ],
    deps = [
        "//trpc/naming/consul/config:consul_naming_conf",
        "//trpc/transport/common/http:curl_http",
        "@com_github_jbeder_yaml_cpp//:yaml-cpp",
        "@com_github_tencent_rapidjson//:rapidjson",
        "@trpc_cpp//trpc/codec/trpc:trpc",
        "@trpc_cpp//trpc/common/config:trpc_config",
        "@trpc_cpp//trpc/naming:selector",
        "@trpc_cpp//trpc/naming:selector_factory",
        "@trpc_cpp//trpc/naming:load_balance_factory",
        "@trpc_cpp//trpc/naming/common/util:utils_help",
        "@trpc_cpp//trpc/naming/common/util/loadbalance/polling:polling_load_balance",
        "@trpc_cpp//trpc/runtime/common:periphery_task_scheduler",
        "@trpc_cpp//trpc/util:string_helper",
        "@trpc_cpp//trpc/util:time",
        "@trpc_cpp//trpc/util/log:logging",
    ],
)

cc_library(
    name = "consul_selector_filter",
    srcs = [],
    hdrs = ["consul_selector_filter.h"],
    visibility = [
        "//visibility:public",
    ],
    deps = [
        "@trpc_cpp//trpc/common/config:trpc_config",
        "@trpc_cpp//trpc/filter:filter",
    ] + select({
        "//conditions:default": [
            "@trpc_cpp//trpc/naming:selector_workflow",
        ],
    })  
)

cc_test(
    name = "consul_selector_test",
    srcs = ["consul_selector_test.cc"],
    data = ["//trpc/naming/consul/testing:consul_test.yaml",],
    deps = [
        ":consul_selector",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "@trpc_cpp//trpc/common:trpc_plugin",
        "@trpc_cpp//trpc/future:future_utility",
        "@trpc_cpp//trpc/naming/common/util/loadbalance/polling:polling_load_balance",
    ],
)

cc_library(
    name = "consul_registry",
    srcs = [
        "consul_registry.cc",
    ],
    hdrs = [
        "consul.h",
        "consul_registry.h",
    ],
    visibility = [
        "//visibility:public",
    ],
    deps = [
        "//trpc/naming/consul/config:consul_naming_conf",
        "//trpc/transport/common/http:curl_http",
        "@com_github_jbeder_yaml_cpp//:yaml-cpp",
        "@com_github_tencent_rapidjson//:rapidjson",
        "@trpc_cpp//trpc/common/config:trpc_config",
        "@trpc_cpp//trpc/filter:filter_manager",
        "@trpc_cpp//trpc/naming:registry",
        "@trpc_cpp//trpc/naming:registry_factory",
        "@trpc_cpp//trpc/util/log:logging",
    ] + select({
        "//conditions:default": [
        ],
    }),
)

cc_test(
    name = "consul_registry_test",
    srcs = ["consul_registry_test.cc"],
    data = ["//trpc/naming/consul/testing:consul_test.yaml",],
    deps = [
        ":consul_registry",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "@trpc_cpp//trpc/common:trpc_plugin",
        "@trpc_cpp//trpc/common/config:trpc_config",
        "@trpc_cpp//trpc/naming/common/util/loadbalance/polling:polling_load_balance",
    ],
)

cc_library(
    name = "consul_selector_api",
    srcs = ["consul_selector_api.cc"],
    hdrs = ["consul_selector_api.h"],
    deps = [
        "//trpc/naming/consul:consul_selector",
        "//trpc/naming/consul:consul_selector_filter",
        "@trpc_cpp//trpc/common:trpc_plugin",
        "@trpc_cpp//trpc/naming:selector_factory",
    ],
)

cc_library(
    name = "consul_registry_api",
    srcs = ["consul_registry_api.cc"],
    hdrs = ["consul_registry_api.h"],
    deps = [
        "//trpc/naming/consul:consul_registry",
        "@trpc_cpp//trpc/common:trpc_plugin",
    ],
)
