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
/// @brief IP+端口信息
class API InetAddress {
public:
  /// @brief 构造函数
  /// @param ip ip地址
  /// @param port 端口号
  explicit InetAddress(const std::string &ip = "127.0.0.1", uint16_t port = 16);
  
  /// @brief 构造函数
  /// @param addr socket地址信息
  explicit InetAddress(const sockaddr_in &addr);

  /// @brief 将IP转换成字符串
  /// @return IP对应字符串
  const std::string toIp() const;

  /// @brief 端口对应整数
  /// @return 端口信息
  const uint16_t toPort() const;

  /// @brief 转换成字符串的IP和端口信息
  /// @return IP和端口信息
  const std::string toIpPort() const;

  /// @brief 获取socket地址
  /// @return socket地址
  const sockaddr_in *sockAddr() const {
    return &sock_addr_;
  }

  /// @brief 设置socket地址
  /// @param addr socket地址
  void setSockAddr(const sockaddr_in &addr) {
    sock_addr_ = addr;
  }

private:
  sockaddr_in sock_addr_;
};

NAMESPACE_END

