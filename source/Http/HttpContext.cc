/*
 * @Author: chenjingyu
 * @Date: 2024-09-18 10:55:54
 * @Contact: 2458006466@qq.com
 * @Description: HttpContext
 */
#include "Http/HttpContext.h"
#include "Base/Buffer.h"
#include "Base/Logger.h"
#include <regex>

NAMESPACE_BEGIN
HttpContext::HttpContext() : state_(kExpectRequestLine) {

}

HttpContext::~HttpContext() = default;

bool HttpContext::ParseRequestLine(const std::string &line) {
  std::regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
  std::smatch match;
  if (std::regex_match(line, match, pattern)) {
    req_.setMethod(match[1].str());
    req_.setPath(match[2].str());
    req_.setVersion(match[3].str());
    state_ = kExpectHeaders;
    return true;
  }
  return false;
}

void HttpContext::ParseHeaders(const std::string &line) {
  std::regex pattern("^([^:]*): ?(.*)$");
  std::smatch match;
  if (std::regex_match(line, match, pattern)) {
    req_.AddHeader(match[1].str(), match[2].str());
  } else {
    state_ = kExpectBody;
  }
}

void HttpContext::ParseBody(const std::string &line) {
  req_.setBody(line);
  // TODO: ParsePost
  state_ = kGotAll;
}


bool HttpContext::ParseRequest(Buffer *buffer, Timestamp recv_time) {
  if (buffer->ReadableBytes() <= 0) {
    return false;
  }
  while (buffer->ReadableBytes() > 0 && state_ != kGotAll) {
    const char *crlf = buffer->FindCRLF();
    if (!crlf) {
     break;
    }
    std::string line(buffer->peek(), crlf);
    switch (state_) {
      case kExpectRequestLine:
        if (!ParseRequestLine(line)) {
          return false;
        }
        break;
      case kExpectHeaders:
        ParseHeaders(line);
        if (buffer->ReadableBytes() <= 2) {
          state_ = kGotAll;
        }
        break;
      case kExpectBody:
        ParseBody(line);
        break;
      default:
        break;
    }
    if (crlf == buffer->beginWrite()) {
      break;
    }
    buffer->RetrieveUtil(crlf + 2);
  }
  LogDebug("[{}], [{}], [{}]", req_.methodString(), req_.path(), req_.version());
  return true;
}

NAMESPACE_END
