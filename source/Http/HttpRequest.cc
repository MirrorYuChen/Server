/*
 * @Author: chenjingyu
 * @Date: 2024-09-18 10:23:44
 * @Contact: 2458006466@qq.com
 * @Description: HttpRequest
 */
#include "Http/HttpRequest.h"
#include "Base/Logger.h"

NAMESPACE_BEGIN
const std::unordered_map<std::string, HttpRequest::Method> StringToMethod = {
  {"GET", HttpRequest::kGet},
  {"POST", HttpRequest::kPost},
  {"HEAD", HttpRequest::kHead},
  {"PUT", HttpRequest::kPut},
  {"DELETE", HttpRequest::kDelete}
};

const std::unordered_map<HttpRequest::Method, std::string> MethodToString = {
  {HttpRequest::kGet, "GET"},
  {HttpRequest::kPost, "POST"},
  {HttpRequest::kHead, "HEAD"},
  {HttpRequest::kPut, "PUT"},
  {HttpRequest::kDelete, "DELETE"}
};

HttpRequest::HttpRequest() : method_(kInvalid), version_("Unknown") {

}

HttpRequest::~HttpRequest() = default;

bool HttpRequest::setMethod(const std::string &method_str) {
  auto iter = StringToMethod.find(method_str);
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

void HttpRequest::setPath(const std::string &path) {
  path_ = path;
}

void HttpRequest::setQuery(const std::string &query) {
  query_ = query;
}

void HttpRequest::AddHeader(const std::string &key, const std::string &value) {
  headers_[key] = value;
}

const std::string HttpRequest::getHeader(const std::string &field) const {
  auto iter = headers_.find(field);
  if (iter != headers_.end()) {
    return iter->second;
  }
  return "";
}

void HttpRequest::Reset() {
  method_ = kInvalid;
  version_ = "Unknown";
  path_.clear();
  body_.clear();
  query_.clear();
  recv_time_ = Timestamp::invalid();
  headers_.clear();
}

const bool HttpRequest::IsKeepAlive() const {
  if (getHeader("Connection") == "keep-alive" && version() == "1.1") {
    return true;
  }
  return false;
}

NAMESPACE_END
