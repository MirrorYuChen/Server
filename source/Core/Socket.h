/*
 * @Author: chenjingyu
 * @Date: 2024-09-10 14:05:59
 * @Contact: 2458006466@qq.com
 * @Description: Socket
 */
#pragma once

#include "Api.h"

NAMESPACE_BEGIN
class InetAddress;
class Socket {
public:
  explicit Socket(int sockfd);
  ~Socket();

  const int sockfd() const {
    return sockfd_;
  }

  void BindAddress(const InetAddress &local_addr);
  void Listen();
  int Accept(InetAddress *peer_addr);

  void ShutdownWrite();

  // 设置为非阻塞
  void setNonBlocking();

  // 设置地址复用
  void setReuseAddr(bool on);

  // 设置端口复用
  void setReusePort(bool on);
  
  // 设置长连接
  void setKeepAlive(bool on);

private:
  const int sockfd_;
};

NAMESPACE_END

