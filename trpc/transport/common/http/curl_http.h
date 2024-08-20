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

#include <curl/curl.h>
#include <curl/easy.h>

#include <map>
#include <memory>
#include <string>

#include "trpc/transport/common/http/core.h"

namespace trpc::curl_http {

using CurlHttpHeaders = std::map<std::string, std::string>;
using CurlHttpSList = struct curl_slist;
using CurlHttpSListPtr = std::shared_ptr<CurlHttpSList>;

struct CurlHttpResponse {
  // Code of curl execution
  int code{-1};

  // Error message
  std::string err_msg{""};

  // Code of response, e.g, 2xx, 3xx, 4xx, 5xx.
  int64_t response_code{500};

  // Body of response
  std::string body{""};

  // Path of body saved in file.
  std::string body_path{""};

  // Flag to indicate body was stored in file.
  unsigned stored_in_file : 1;

  CurlHttpResponse() : code(-1), err_msg(""), response_code(500), body(""), body_path(""), stored_in_file(0) {}

  ~CurlHttpResponse() = default;
};

using CurlHttpResponsePtr = std::shared_ptr<CurlHttpResponse>;

class CurlHttp {
 public:
  struct Options {
    //
    // Timeout in millisecond of connecting to server.
    // Set zero to switch to the default built-in connection timeout-300 seconds.
    //
    int64_t connection_timeout{3000L};

    //
    // The maximum time in milliseconds that you allow the libcurl transfer operation to take.
    // Set to zero to switch to the default built-in connection timeout - 300 seconds.
    //
    int64_t timeout{10000L};

    // HTTP request headers
    CurlHttpHeaders request_headers;

    //
    // This option determines whether curl verifies the authenticity of the peer's certificate.
    // A value of 1 means curl verifies; 0 (zero) means it doesn't.
    //
    unsigned int insecure : 1;
  };

  //
  // @brief This callback function gets called by libcurl as soon as there is data received
  // -- that needs to be saved.
  // For most transfers, this callback gets called many times and each invoke delivers
  // -- another chunk of data.
  // @param src points to the delivered data.
  // @param nmemb is size of the delivered data.
  // @param size is always 1.
  // @param dst points to user data which stores delivered data.
  // @return int, return the number of bytes actually taken care of.
  //
  // Reference :https://curl.haxx.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
  //
  static int CurlWriteCallback(char* src, size_t size, size_t nmemb, std::string* dst);

 public:
  int Init();
  void Destroy();

  // HTTP GET
  CurlHttpResponsePtr Get(const std::string& url);

  // HTTP POST
  CurlHttpResponsePtr Post(const std::string& url, const std::string& body);

  // HTTP PUT
  CurlHttpResponsePtr Put(const std::string& url, const std::string& body);

 public:
  CurlHttp();
  ~CurlHttp();

  void SetConnectionTimeout(int64_t timeout) { curl_options_.connection_timeout = timeout; }
  int64_t GetConnectionTimeout() { return curl_options_.connection_timeout; }

  void SetTimeout(int64_t timeout) { curl_options_.timeout = timeout; }
  int64_t GetTimeout() { return curl_options_.timeout; }

  void SetInsecure(unsigned int insecure) { curl_options_.insecure = insecure; }
  unsigned int GetInsecure() { return curl_options_.insecure; }

  void SetRequestHeader(const std::string& name, const std::string& value) {
    curl_options_.request_headers[name] = value;
  }
  int GetRequestHeader(const std::string& name, std::string* value) {
    if (!value) return kError;
    auto iter = curl_options_.request_headers.find(name);
    if (iter != curl_options_.request_headers.end()) {
      *value = iter->second;
      return kOk;
    }
    return kError;
  }

 private:
  CurlHttpSList* CreateCurlSList(const CurlHttpHeaders& http_headers);
  void DoCurlEasySetOption();
  void SetCurlOption(const std::string& url, CurlHttpResponsePtr& response, CurlHttpSList* headers);

 private:
  CURL* curl_;
  CurlHttp::Options curl_options_;
  char* curl_err_buf_;
};

using CurlHttpOptions = CurlHttp::Options;
using CurlHttpPtr = std::shared_ptr<CurlHttp>;
using CurlHttpOptionsPtr = std::shared_ptr<CurlHttpOptions>;

}  // namespace trpc::curl_http
