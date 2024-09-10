/*
 * @Author: chenjingyu
 * @Date: 2024-09-10 14:13:21
 * @Contact: 2458006466@qq.com
 * @Description: InetAddress
 */
#pragma once

#include "Api.h"
#include <arpa/inet.h>
#include <string>

NAMESPACE_BEGIN
class API InetAddress {
public:
  explicit InetAddress(const std::string &ip = "127.0.0.1", uint16_t port = 16);
  explicit InetAddress(const sockaddr_in &addr);

  const std::string toIp() const;
  const uint16_t toPort() const;
  const std::string toIpPort() const;

  const sockaddr_in *sockAddr() const {
    return &sock_addr_;
  }

  void setSockAddr(const sockaddr_in &addr) {
    sock_addr_ = addr;
  }

private:
  sockaddr_in sock_addr_;
};

NAMESPACE_END

