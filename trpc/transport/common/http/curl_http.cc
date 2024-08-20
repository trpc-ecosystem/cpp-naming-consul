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

#include "trpc/transport/common/http/curl_http.h"

#include <utility>

namespace trpc::curl_http {

CurlHttp::CurlHttp() : curl_(nullptr), curl_err_buf_(nullptr) {
  curl_options_.connection_timeout = 3000L;
  curl_options_.timeout = 10000L;
  curl_options_.request_headers.insert(std::pair<std::string, std::string>("User-Agent", "Curl-HTTP-Wrapper/0.1.0"));
  curl_options_.insecure = 0;

  curl_err_buf_ = new char[CURL_ERROR_SIZE];
}

CurlHttp::~CurlHttp() { Destroy(); }

int CurlHttp::CurlWriteCallback(char* src, size_t size, size_t nmemb, std::string* dst) {
  if (!dst) return 0;

  uint32_t total_size = size * nmemb;
  dst->append(src, total_size);

  return total_size;
}

int CurlHttp::Init() {
  if (!curl_) {
    curl_ = curl_easy_init();
    if (!curl_) return kError;
  }

  return kOk;
}

void CurlHttp::Destroy() {
  if (curl_) {
    curl_easy_cleanup(curl_);
    curl_ = nullptr;
  }

  if (curl_err_buf_) {
    delete[] curl_err_buf_;
    curl_err_buf_ = nullptr;
  }

  return;
}

CurlHttpResponsePtr CurlHttp::Get(const std::string& url) {
  if (!curl_) return nullptr;

  CurlHttpResponsePtr response = std::make_shared<CurlHttpResponse>();

  // Set default options of curl_easy.
  DoCurlEasySetOption();

  // Set http request headers
  CurlHttpSList* headers = CreateCurlSList(curl_options_.request_headers);
  if (headers) curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);

  // Set write callback
  curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, CurlHttp::CurlWriteCallback);
  curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &(response->body));

  // Set target url
  curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());

  // Do curl http://xxx.com/yy/to/path
  CURLcode curl_code = curl_easy_perform(curl_);
  response->code = static_cast<int>(curl_code);

  //
  // Get response code of HTTP
  // The stored value will be zero if no server response code has been received.
  // Note that a proxy's CONNECT response should be read with CURLINFO_HTTP_CONNECTCODE
  // -- and not this.
  //
  curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &(response->response_code));

  if (CURLE_OK != curl_code) {
    response->err_msg.append(curl_err_buf_);
  }

  curl_slist_free_all(headers);
  return response;
}

CurlHttpResponsePtr CurlHttp::Post(const std::string& url, const std::string& body) {
  if (!curl_) return nullptr;

  CurlHttpResponsePtr response = std::make_shared<CurlHttpResponse>();

  CurlHttpSList* headers = CreateCurlSList(curl_options_.request_headers);
  SetCurlOption(url, response, headers);

  // Set method to HTTP POST
  curl_easy_setopt(curl_, CURLOPT_POST, 1L);

  // Set post body
  curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, body.c_str());
  curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, body.size());

  // Do curl http://xxx.com/yy/to/path
  CURLcode curl_code = curl_easy_perform(curl_);
  response->code = static_cast<int>(curl_code);

  //
  // Get response code of HTTP
  // The stored value will be zero if no server response code has been received.
  // Note that a proxy's CONNECT response should be read with CURLINFO_HTTP_CONNECTCODE
  // -- and not this.
  //
  curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &(response->response_code));

  if (CURLE_OK != curl_code) {
    response->err_msg.append(curl_err_buf_);
  }

  curl_slist_free_all(headers);
  return response;
}

CurlHttpResponsePtr CurlHttp::Put(const std::string& url, const std::string& body) {
  if (!curl_) return nullptr;

  CurlHttpResponsePtr response = std::make_shared<CurlHttpResponse>();

  CurlHttpSList* headers = CreateCurlSList(curl_options_.request_headers);
  SetCurlOption(url, response, headers);

  // Set method to HTTP POST
  curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "PUT");

  // Set post body
  curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, body.c_str());
  curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, body.size());

  // Do curl http://xxx.com/yy/to/path
  CURLcode curl_code = curl_easy_perform(curl_);
  response->code = static_cast<int>(curl_code);

  //
  // Get response code of HTTP
  // The stored value will be zero if no server response code has been received.
  // Note that a proxy's CONNECT response should be read with CURLINFO_HTTP_CONNECTCODE
  // -- and not this.
  //
  curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &(response->response_code));

  if (CURLE_OK != curl_code) {
    response->err_msg.append(curl_err_buf_);
  }

  curl_slist_free_all(headers);
  return response;
}

CurlHttpSList* CurlHttp::CreateCurlSList(const CurlHttpHeaders& http_headers) {
  if (http_headers.empty()) return nullptr;

  CurlHttpSList *headers = nullptr, *tmp_headers = nullptr;
  for (auto& [header_name, header_value] : http_headers) {
    if (!header_name.empty() && !header_value.empty()) {
      std::string header_str(header_name + ": " + header_value);
      tmp_headers = curl_slist_append(headers, header_str.c_str());
      // something went wrong
      if (!tmp_headers) {
        return headers;
      }
      headers = tmp_headers;
    }
  }
  tmp_headers = nullptr;
  return headers;
}

void CurlHttp::DoCurlEasySetOption() {
  if (!curl_) return;

  // Set connection timeout.
  curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT, curl_options_.connection_timeout);
  curl_easy_setopt(curl_, CURLOPT_TIMEOUT, curl_options_.timeout);

  // Set flag to indicate checking SSL security or not.
  int64_t ssl_verify_peer = curl_options_.insecure ? 0L : 1L;
  curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, ssl_verify_peer);
  curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, ssl_verify_peer);

  // Set error buffer for curl..
  if (curl_err_buf_) curl_easy_setopt(curl_, CURLOPT_ERRORBUFFER, curl_err_buf_);

  // Set ignore signal
  curl_easy_setopt(curl_, CURLOPT_NOSIGNAL, 1L);

  // Allow redirection, follow HTTP 3xx redirects
  curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);

  //
  // The set number will be the redirection limit amount.
  // If that many redirections have been followed, the next redirect will cause an
  // -- error (CURLE_TOO_MANY_REDIRECTS).
  // This option only makes sense if the CURLOPT_FOLLOWLOCATION is used at the same time.
  //
  // Setting the limit to 0 will make libcurl refuse any redirect.
  // Set it to -1 for an infinite number of redirects.
  //
  curl_easy_setopt(curl_, CURLOPT_MAXREDIRS, 10L);

  return;
}

void CurlHttp::SetCurlOption(const std::string& url, CurlHttpResponsePtr& response, CurlHttpSList* headers) {
  // Set default options of curl_easy.
  DoCurlEasySetOption();

  // Set http request headers
  if (headers) curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);

  // Set write callback
  curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, CurlHttp::CurlWriteCallback);
  curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &(response->body));

  // Set target url
  curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
}

}  // namespace trpc::curl_http
