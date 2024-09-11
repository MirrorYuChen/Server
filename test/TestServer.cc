/*
 * @Author: chenjingyu
 * @Date: 2024-09-11 17:08:08
 * @Contact: 2458006466@qq.com
 * @Description: TestServer
 */
#include "Core/TcpServer.h"
#include "Base/Logger.h"

NAMESPACE_BEGIN
class EchoServer {
public:
  EchoServer(EventLoop *loop, const InetAddress &addr, const std::string &name)
      : server_(loop, addr, name), loop_(loop) {
    server_.setConnectionCallback(
      std::bind(&EchoServer::onConnection, this, std::placeholders::_1)
    );

    server_.setMessageCallback( 
      std::bind(
        &EchoServer::onMessage, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3
      )
    );

    // 设置合适的subloop线程数量
    server_.setThreadNum(3);
  }

  void Start() { server_.Start(); }

private:
  // 连接建立或断开的回调函数
  void onConnection(const TcpConnectionPtr &conn) {
    if (conn->connected()) {
      LogStream(LInfo) << "Connection UP : " << conn->peerAddress().toIpPort().c_str();
    } else {
      LogStream(LInfo) << "Connection DOWN : " << conn->peerAddress().toIpPort().c_str();
    }
  }

  // 可读写事件回调
  void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time) {
    std::string msg = buf->RetrieveAllAsString();
    LogStream(LInfo) << conn->name() << " echo " << msg.size() << " bytes, data received at " << time.toFormattedString();
    conn->Send(msg);
    // conn->shutdown();   // 关闭写端 底层响应EPOLLHUP => 执行closeCallback_
  }

  EventLoop *loop_;
  TcpServer server_;
};

NAMESPACE_END

using namespace NAMESPACE;
int main() {
  LogStream(LInfo) << "pid = " << getpid();
  EventLoop loop;
  InetAddress addr("127.0.0.1", 8080);
  EchoServer server(&loop, addr, "EchoServer");
  server.Start();
  loop.Loop();

}