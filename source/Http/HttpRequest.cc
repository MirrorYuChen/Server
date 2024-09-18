/*
 * @Author: chenjingyu
 * @Date: 2024-09-18 10:23:44
 * @Contact: 2458006466@qq.com
 * @Description: HttpRequest
 */
#include "Http/HttpRequest.h"

NAMESPACE_BEGIN
const std::unordered_map<std::string, HttpRequest::Method> StringToMethod = {
  {"Get", HttpRequest::kGet},
  {"POST", HttpRequest::kPost},
  {"HEAD", HttpRequest::kHead},
  {"PUT", HttpRequest::kPut},
  {"DELETE", HttpRequest::kDelete}
};

const std::unordered_map<HttpRequest::Method, std::string> MethodToString = {
  {HttpRequest::kGet, "Get"},
  {HttpRequest::kPost, "POST"},
  {HttpRequest::kHead, "HEAD"},
  {HttpRequest::kPut, "PUT"},
  {HttpRequest::kDelete, "DELETE"}
};

HttpRequest::HttpRequest() : method_(kInvalid), version_(kUnknown) {

}

HttpRequest::~HttpRequest() = default;

bool HttpRequest::setMethod(const char *start, const char *end) {
  std::string key(start, end);
  auto iter = StringToMethod.find(key);
  if (iter != StringToMethod.end()) {
    method_ = iter->second;
    return true;
  } else {
    method_ = kInvalid;
    return false;
  }
}

const char *HttpRequest::methodString() const {
  auto iter = MethodToString.find(method_);
  if (iter != MethodToString.end()) {
    return iter->second.data();
  } else {
    return "UNKNOWN";
  }
}

void HttpRequest::setPath(const char *start, const char *end) {
  path_.clear();
  path_.assign(start, end);
}

void HttpRequest::setQuery(const char *start, const char *end) {
  query_.clear();
  query_.assign(start, end);
}

void HttpRequest::AddHeader(const char *start, const char *colon, const char *end) {
  std::string field(start, end);
  ++colon;
  // 1.从前往后跳过空格
  while (colon < end && isspace(*colon)) {
    ++colon;
  }
  std::string value(colon, end);
  // 2.从后往前跳过空格
  while (!value.empty() && isspace(value[value.size() - 1])) {
    value.resize(value.size() - 1);
  }
  headers_[field] = value;
}

const std::string HttpRequest::getHeader(const std::string &field) const {
  auto iter = headers_.find(field);
  if (iter != headers_.end()) {
    return iter->second;
  }
  return "";
}

void HttpRequest::Swap(HttpRequest &other) {
  std::swap(method_, other.method_);
  std::swap(version_, other.version_);
  path_.swap(other.path_);
  query_.swap(other.query_);
  std::swap(recv_time_, other.recv_time_);
  headers_.swap(other.headers_);
}

NAMESPACE_END
