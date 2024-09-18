/*
 * @Author: chenjingyu
 * @Date: 2024-09-18 11:34:45
 * @Contact: 2458006466@qq.com
 * @Description: HttpServer
 */
#include "Http/HttpServer.h"
#include "Http/HttpContext.h"
#include <memory>

NAMESPACE_BEGIN
/**
 * 默认的http回调函数
 * 设置响应状态码，响应信息并关闭连接
 */
void DefaultHttpCallback(const HttpRequest &, HttpResponse *resp) {
  resp->setStatusCode(HttpResponse::kNotFound);
  resp->setStatusMessage("Not Found");
  resp->setCloseConnection(true);
}

HttpServer::HttpServer(EventLoop *loop, const InetAddress &listen_addr,
                       const std::string &name, TcpServer::Option option)
    : server_(loop, listen_addr, name, option),
      cb_(DefaultHttpCallback) {
  server_.setConnectionCallback(
    std::bind(&HttpServer::onConnection, this, std::placeholders::_1)
  );
  server_.setMessageCallback(
    std::bind(
      &HttpServer::onMessage, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3
    )
  );
  server_.setThreadNum(4);
}

void HttpServer::Start() {
  LogStream(LInfo) << "HttpServer[" << server_.name() << "] starts listening on " << server_.ipPort();
  server_.Start();
}

void HttpServer::onConnection(const TcpConnectionPtr &conn) {
  if (conn->connected()) {
    LogStream(LInfo) << "new Connection arrived";
  } else {
    LogStream(LInfo) << "Connection closed";
  }
}
void HttpServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf,
                           Timestamp recv_time) {
  std::unique_ptr<HttpContext> context(new HttpContext);


  // 进行状态机解析
  // 错误则发送 BAD REQUEST 半关闭
  if (!context->ParseRequest(buf, recv_time)) {
    LogStream(LInfo) << "ParseRequest failed!";
    conn->Send("HTTP/1.1 400 Bad Request\r\n\r\n");
    conn->Shutdown();
  }

  // 如果成功解析
  if (context->gotAll()) {
    LogStream(LInfo) << "ParseRequest success!";
    onRequest(conn, context->request());
    context->Reset();
  }
}

void HttpServer::onRequest(const TcpConnectionPtr &conn,
                           const HttpRequest &req) {
  const std::string &connection = req.getHeader("Connection");
  bool close = connection == "close" || (
    req.version() == HttpRequest::kHttp10 && 
    connection != "Keep-Alive"
  );
  close = true;
  HttpResponse response(close);
  cb_(req, &response);
  Buffer buf;
  response.AppendToBuffer(&buf);
  std::string str = buf.RetrieveAllAsString();
  LogDebug("response: {}.", str);
  conn->Send(str);
  if (response.closeConnection()) {
    conn->Shutdown();
  }
}

NAMESPACE_END