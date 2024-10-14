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
class API HttpContext {
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
    req_.Reset();
  }

  const HttpRequest &request() const {
    return req_;
  }

  HttpRequest &request() {
    return req_;
  }

private:
  bool ParseRequestLine(const std::string &line);
  void ParseHeaders(const std::string &line);
  void ParseBody(const std::string &line);
  void ParsePath();

private:
  HttpRequestParseState state_;
  HttpRequest req_;
};

NAMESPACE_END
