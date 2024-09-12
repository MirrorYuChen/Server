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
/// @brief 基本socket封装类
class API Socket {
public:
  /// @brief 构造函数
  /// @param sockfd 创建socket的文件描述符
  explicit Socket(int sockfd);
  ~Socket();

  /// @brief 获取当前socket的文件描述符
  /// @return 当前socket的文件描述符
  const int sockfd() const {
    return sockfd_;
  }

  /// @brief 绑定待监听本地地址端口
  /// @param local_addr 待监听本地地址端口
  void BindAddress(const InetAddress &local_addr);

  /// @brief 启动监听
  void Listen();

  /// @brief 根据监听到客户端信息创建一个socket
  /// @param peer_addr 监听到客户端地址端口信息
  /// @return 创建的客户端socket文件描述符
  int Accept(InetAddress *peer_addr);

  /// @brief 关闭socket写功能
  void ShutdownWrite();

  /// @brief 设置socket为非阻塞
  void setNonBlocking();

  /// @brief 设置socket为地址复用
  void setReuseAddr(bool on);

  /// @brief 设置socket为端口复用
  void setReusePort(bool on);

  /// @brief 设置socket为长连接
  void setKeepAlive(bool on);

private:
  const int sockfd_;
};

NAMESPACE_END

