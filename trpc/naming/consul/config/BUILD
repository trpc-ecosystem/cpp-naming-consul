# Description: trpc-cpp.

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "consul_naming_conf",
    srcs = ["consul_naming_conf.cc"],
    hdrs = ["consul_naming_conf.h"],
    deps = [
        "@com_github_jbeder_yaml_cpp//:yaml-cpp",
        "@trpc_cpp//trpc/client:client_context",
        "@trpc_cpp//trpc/util/log:logging",
    ],
)
