# Description: trpc-cpp

licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "core",
    srcs = [],
    hdrs = ["core.h"],
    visibility = [
        "//visibility:public",
    ],
    deps = [],
)

cc_library(
    name = "curl_http",
    srcs = ["curl_http.cc"],
    hdrs = ["curl_http.h"],
    visibility = [
        "//visibility:public",
    ],
    deps = [
        ":core",
        "@local_curl//:libcurl",
    ],
)
