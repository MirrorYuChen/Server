/*
 * @Author: chenjingyu
 * @Date: 2024-09-18 10:55:54
 * @Contact: 2458006466@qq.com
 * @Description: HttpContext
 */
#include "Http/HttpContext.h"
#include "Base/Buffer.h"
#include "Base/Logger.h"

NAMESPACE_BEGIN
HttpContext::HttpContext() : state_(kExpectRequestLine) {

}

HttpContext::~HttpContext() = default;

bool HttpContext::ParseRequestLine(const char *begin, const char *end) {
  bool succeed = false;
  const char *start = begin;
  const char *space = std::find(start, end, ' ');

  // 1.成功解析出method
  if (space != end && req_.setMethod(start, space)) {
    LogDebug("method: {}.", req_.methodString());
    // 1.1 跳过并查找下一个空格
    start = space + 1;
    space = std::find(start, end, ' ');
    if (space == end) {
      return succeed;
    }

    // 1.2 解析path信息，查询请求参数
    const char *question = std::find(start, space, '?') ;
    if (question != space) {
      req_.setPath(start, question);
      req_.setQuery(question, space);
    } else {
      req_.setPath(start, space);
    }
    LogDebug("path: {}, query: {}.", req_.path(), req_.query());
    start = space + 1;

    // 1.3 解析version信息
    succeed = (end - start == 8 && std::equal(start, end - 1, "HTTP/1."));
    if (succeed) {
      if (*(end - 1) == '1') {
        req_.setVersion(HttpRequest::kHttp11);
      } else if (*(end - 1) == '0') {
        req_.setVersion(HttpRequest::kHttp10);
      } else {
        succeed = false;
      }
    }
    LogDebug("version: {}.", req_.version());
  }
  return succeed;
}


bool HttpContext::ParseRequest(Buffer *buffer, Timestamp recv_time) {
  bool ok = false;
  bool has_more = true;

  while (has_more) {
    if (state_ == kExpectRequestLine) {
      const char *crlf = buffer->FindCRLF();
      if (crlf) {
        ok = ParseRequestLine(buffer->peek(), crlf);
        if (ok) {
          req_.setReceiveTime(recv_time);
          buffer->RetrieveUtil(crlf + 2);
          state_ = kExpectHeaders;
        } else {
          has_more = false;
        }
      } else {
        has_more = false;
      }
    } else if (state_ == kExpectHeaders) {
      const char *crlf = buffer->FindCRLF();
      if (crlf) {
        const char *colon = std::find(buffer->peek(), crlf, ':');
        if (colon != crlf) {
          req_.AddHeader(buffer->peek(), colon, crlf);
        } else {
          state_ = kGotAll;
          has_more = false;
        }
        buffer->RetrieveUtil(crlf + 2);
      } else {
        has_more = false;
      }
    } else if (state_ == kExpectBody) {
      // TODO:
    }
  }
  const auto &headers = req_.headers();
  for (const auto &header : headers) {
    LogDebug("header => key: {}, value: {}.", header.first, header.second);
  }
  return ok;
}

NAMESPACE_END
