/*
 * @Author: chenjingyu
 * @Date: 2024-09-18 11:20:18
 * @Contact: 2458006466@qq.com
 * @Description: HttpResponse
 */
#include "Http/HttpResponse.h"
#include "Base/Buffer.h"

NAMESPACE_BEGIN
HttpResponse::HttpResponse(bool close) : 
  status_code_(kUnknown), close_connection_(close) {

}

HttpResponse::~HttpResponse() = default;

void HttpResponse::AppendToBuffer(Buffer *output) const {
  std::string str = "HTTP/1.1 " + std::to_string(status_code_) + " " + status_msg_ + "\r\n";
  output->Append(str);
  if (close_connection_) {
    output->Append("Connection: close\r\n");
  } else {
    str.clear();
    str = "Content-Length: " + std::to_string(body_.size()) + "\r\n";
    output->Append(str);
    output->Append("Connection: Keep-Alive\r\n");
  }
  
  for (const auto &header : headers_) {
    output->Append(header.first);
    output->Append(": ");
    output->Append(header.second);
    output->Append("\r\n");
  }
  output->Append("\r\n");
  output->Append(body_);
}

NAMESPACE_END
