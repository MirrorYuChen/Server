/*
 * @Author: chenjingyu
 * @Date: 2024-09-10 14:13:21
 * @Contact: 2458006466@qq.com
 * @Description: InetAddress
 */
#include "Core/InetAddress.h"
#include <netinet/in.h>
#include <string.h>

NAMESPACE_BEGIN
InetAddress::InetAddress(const std::string &ip, uint16_t port) {
  memset(&sock_addr_, 0, sizeof(sock_addr_));
  sock_addr_.sin_family = AF_INET;
  sock_addr_.sin_port = htons(port);
  sock_addr_.sin_addr.s_addr = inet_addr(ip.c_str());
}

InetAddress::InetAddress(const sockaddr_in &addr) : sock_addr_(addr) {

}

const std::string InetAddress::toIp() const {
  char buf[64] = {0};
  inet_ntop(AF_INET, &sock_addr_.sin_addr, buf, sizeof(buf));
  return buf;
}

const uint16_t InetAddress::toPort() const {
  return ntohs(sock_addr_.sin_port);
}

const std::string InetAddress::toIpPort() const {
  return toIp() + ":" + std::to_string(toPort());
}

NAMESPACE_END
