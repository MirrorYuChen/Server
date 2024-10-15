/*
 * @Author: chenjingyu
 * @Date: 2024-09-18 11:34:39
 * @Contact: 2458006466@qq.com
 * @Description: HttpServer
 */
#pragma once

#include "Api.h"
#include "Base/Logger.h"
#include "Core/TcpServer.h"
#include "Http/HttpRequest.h"
#include "Http/HttpResponse.h"
#include <functional>
#include <string>

NAMESPACE_BEGIN
class API HttpServer {
public:
  using HttpCallback = std::function<void(const HttpRequest &, HttpResponse *)>;
  HttpServer(EventLoop *loop, const InetAddress &listen_addr,
             const std::string &name, const std::string &root_path,
             TcpServer::Option option = TcpServer::kNoReusePort);

  void InitDatabase(const std::string &url,
    const std::string &user,
    const std::string &passwd,
    const std::string &dbname,
    unsigned int port,
    int max_conn
  );

  EventLoop *getLoop() const { return server_.getLoop(); }

  void Start();

private:
  void onConnection(const TcpConnectionPtr &conn_ptr);
  void onMessage(const TcpConnectionPtr &conn_ptr, Buffer *buffer, Timestamp recv_time);
  void onRequest(const TcpConnectionPtr &conn_ptr, const HttpRequest &req);

  NOT_ALLOWED_COPY(HttpServer)

private:
  TcpServer server_;
  const std::string root_path_;
};

NAMESPACE_END
