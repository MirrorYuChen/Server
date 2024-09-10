/*
 * @Author: chenjingyu
 * @Date: 2024-09-10 14:06:05
 * @Contact: 2458006466@qq.com
 * @Description: Socket
 */
#include "Core/Socket.h"
#include "Base/Logger.h"
#include "Core/InetAddress.h"
#include <unistd.h>

NAMESPACE_BEGIN
Socket::Socket(int sockfd) : sockfd_(sockfd) {}
Socket::~Socket() {
  if (sockfd_ != -1) {
    close(sockfd_);
  }
}

void Socket::BindAddress(const InetAddress &local_addr) {
  int ret = bind(sockfd_, (sockaddr *)local_addr.sockAddr(), sizeof(sockaddr_in));
  CHECK(ret == 0) << "Failed bind socket fd: " << sockfd_;
}

void Socket::Listen() {
  int ret = listen(sockfd_, 1024);
  CHECK(ret == 0) << "Failed listen socket fd: " << sockfd_;
}

int Socket::Accept(InetAddress *perr_addr) {
  sockaddr_in addr;
  socklen_t addr_len = sizeof(addr);
  memset(&addr, 0, sizeof(addr));

  int clnt_sockfd = accept(sockfd_, (sockaddr*) &addr, &addr_len);
  if (clnt_sockfd >= 0) {
    perr_addr->setSockAddr(addr);
  } else {
    LogError("accept4() failed.");
  }
  return clnt_sockfd;
}

void Socket::ShutdownWrite() {
  int ret = shutdown(sockfd_, SHUT_WR);
  if (ret < 0) {
    LogError("ShutdownWrite error.");
  }
}

void Socket::setNonBlocking() {
  fcntl(sockfd_, F_SETFL, fcntl(sockfd_, F_GETFL, 0) | O_NONBLOCK);
}

// 设置地址复用，其实就是可以使用处于Time-wait的端口
void Socket::setReuseAddr(bool on) {
  int optval = on ? 1 : 0;
  setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

// 通过改变内核信息，多个进程可以绑定同一个地址。通俗就是多个服务的ip+port是一样
void Socket::setReusePort(bool on) {
  int optval = on ? 1 : 0;
  setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)); 
}

void Socket::setKeepAlive(bool on) {
  int optval = on ? 1 : 0;
  setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)); 
}

NAMESPACE_END
