/*
 * @Author: chenjingyu
 * @Date: 2024-09-18 11:34:45
 * @Contact: 2458006466@qq.com
 * @Description: HttpServer
 */
#include "Http/HttpServer.h"
#include "Http/HttpContext.h"
#include "Database/ConnectionPool.h"
#include <memory>

NAMESPACE_BEGIN
HttpServer::HttpServer(EventLoop *loop, const InetAddress &listen_addr,
                       const std::string &name, const std::string &root_path, 
                       TcpServer::Option option)
    : server_(loop, listen_addr, name, option), root_path_(root_path) {
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

void HttpServer::InitDatabase(const std::string &url,
  const std::string &user,
  const std::string &passwd,
  const std::string &dbname,
  unsigned int port,
  int max_conn
) {
  ConnectionPool::getInstance()->Init(url, user, passwd, dbname, port, max_conn);
}

void HttpServer::Start() {
  LogInfo("HttpServer [{}] starts listening on {}.", server_.name(), server_.ipPort());
  server_.Start();
}

void HttpServer::onConnection(const TcpConnectionPtr &conn) {
  if (conn->connected()) {
    LogInfo("new Connection arrived");
  } else {
    LogInfo("Connection closed");
  }
}
void HttpServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf,
                           Timestamp recv_time) {
  std::unique_ptr<HttpContext> context(new HttpContext);
  // 进行状态机解析
  // 错误则发送 BAD REQUEST 半关闭
  if (!context->ParseRequest(buf, recv_time)) {
    LogInfo("ParseRequest failed!");
    conn->Send("HTTP/1.1 400 Bad Request\r\n\r\n");
    conn->Shutdown();
  }

  // 如果成功解析
  if (context->gotAll()) {
    LogInfo("ParseRequest success!");
    onRequest(conn, context->request());
    context->Reset();
  }
}

void HttpServer::onRequest(const TcpConnectionPtr &conn,
                           const HttpRequest &req) {
  HttpResponse resp;
  resp.Init(root_path_, req.path(), req.IsKeepAlive());
  LogInfo("Path: {}.", req.path());
  Buffer buffer;
  resp.MakeResponse(&buffer);
  buffer.Append(std::string(resp.file(), resp.fileSize()));
  std::string str = buffer.RetrieveAllAsString();
  conn->Send(str);
  if (!req.IsKeepAlive()) {
    conn->Shutdown();
  }
}

NAMESPACE_END
