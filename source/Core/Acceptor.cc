/*
 * @Author: chenjingyu
 * @Date: 2024-09-11 14:14:56
 * @Contact: 2458006466@qq.com
 * @Description: Acceptor
 */
#include "Core/Acceptor.h"
#include <sys/socket.h>
#include "Base/Logger.h"
#include "Core/InetAddress.h"

NAMESPACE_BEGIN
static int CreateNonBlocking() {
  int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
  CHECK(sockfd != -1) << "Create socket error.";
  return sockfd;
}

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listen_addr, bool reuse_addr, bool reuse_port) :
  loop_(loop),
  accept_socket_(CreateNonBlocking()),
  accept_channel_(loop, accept_socket_.sockfd()),
  listening_(false) {
  LogInfo("Acceptor create nonblocking socket [fd = {}]", accept_channel_.sockfd());
  accept_socket_.setReuseAddr(reuse_addr);
  accept_socket_.setReusePort(reuse_port);
  accept_socket_.BindAddress(listen_addr);

  /**
   * 有新用户连接时，需要执行一个回调函数
   * 因此向封装了accept_socket_的channel注册回调函数
   * 在HandleRead函数中会调用设置的回调函数cb_
   * cb_对应于TcpServer::NewConnection函数
   */
  accept_channel_.setReadCallback(
    std::bind(&Acceptor::HandleRead, this)
  );
}

Acceptor::~Acceptor() {
  // 从poller中删除所有感兴趣事件
  accept_channel_.DisableAll();
  // 删除Poller的ChannnelMap对应部分
  accept_channel_.Remove();
}

void Acceptor::Listen() {
  listening_ = true;
  accept_socket_.Listen();
  // 向poller注册读事件
  accept_channel_.EnableReading();
}

void Acceptor::HandleRead() {
  // 1.接收client的socketfd
  InetAddress peer_addr;
  int connfd = accept_socket_.Accept(&peer_addr);
  // 2.接收到client的连接
  if (connfd != -1) {
    // 2.1 调用设置的连接回调函数：TcpServer::NewConnection
    if (cb_) {
      cb_(connfd, peer_addr);
    } else {
      LogDebug("no newConnectionCallback() function.");
      close(connfd);
    }
  } else {
    LogError("Accept() failed.");
    // 3.当前进程的fd用完，达到单个服务器fd上限
    if (errno == EMFILE) {
      LogError("sockfd reached limit.");
    }
  }
}
NAMESPACE_END
