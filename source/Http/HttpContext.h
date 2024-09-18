/*
 * @Author: chenjingyu
 * @Date: 2024-09-18 10:55:48
 * @Contact: 2458006466@qq.com
 * @Description: HttpContext
 */
#pragma once

#include "Api.h"
#include "Http/HttpRequest.h"

NAMESPACE_BEGIN
class Buffer;
class HttpContext {
public:
  enum HttpRequestParseState {
    kExpectRequestLine,
    kExpectHeaders,
    kExpectBody,
    kGotAll,
  };

  HttpContext();
  ~HttpContext();
  
  bool ParseRequest(Buffer *buffer, Timestamp recv_time);
  bool gotAll() const {
    return state_ == kGotAll;
  }

  void Reset() {
    state_ = kExpectRequestLine;
    HttpRequest dummy;
    req_.Swap(dummy);
  }

  const HttpRequest &request() const {
    return req_;
  }

  HttpRequest &request() {
    return req_;
  }

private:
  bool ParseRequestLine(const char *begin, const char *end);

private:
  HttpRequestParseState state_;
  HttpRequest req_;
};

NAMESPACE_END
