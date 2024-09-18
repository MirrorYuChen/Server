/*
 * @Author: chenjingyu
 * @Date: 2024-09-18 11:20:10
 * @Contact: 2458006466@qq.com
 * @Description: HttpResponce
 */
#pragma once

#include "Api.h"
#include <string>
#include <unordered_map>

NAMESPACE_BEGIN
class Buffer;
class HttpResponse {
public:
  enum HttpStatusCode {
    kUnknown,
    kOK = 200,
    kMovedPermanently = 301,
    kBadRequest = 400,
    kNotFound = 404,
  };

  explicit HttpResponse(bool close);
  ~HttpResponse();

  void setStatusCode(HttpStatusCode code) {
    status_code_ = code;
  }

  void setStatusMessage(const std::string &msg) {
    status_msg_ = msg;
  }

  void setCloseConnection(bool on) {
    close_connection_ = on;
  }

  const bool closeConnection() const {
    return close_connection_;
  }

  void setContentType(const std::string &type) {
    AddHeader("Content-Type", type);
  }

  void AddHeader(const std::string &key, const std::string &value) {
    headers_[key] = value;
  }

  void setBody(const std::string &body) {
    body_ = body;
  }

  void AppendToBuffer(Buffer *output) const;

private:
  std::unordered_map<std::string, std::string> headers_;
  HttpStatusCode status_code_;
  std::string status_msg_;
  bool close_connection_;
  std::string body_;
};


NAMESPACE_END
