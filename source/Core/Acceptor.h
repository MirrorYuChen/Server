/*
 * @Author: chenjingyu
 * @Date: 2024-09-11 14:14:50
 * @Contact: 2458006466@qq.com
 * @Description: Acceptor
 */
#pragma once

#include "Api.h"

#include <functional>

#include "Core/Socket.h"
#include "Core/Channel.h"

NAMESPACE_BEGIN
class EventLoop;
class InetAddress;
/// @brief Acceptor运行在mainLoop中，TcpServer发现
/// Acceptor中有一个新连接，则将Channel分发给一个subLoop
class API Acceptor {
public:
  using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;
  Acceptor(EventLoop *loop, const InetAddress &listen_addr, 
           bool reuse_addr = true, bool reuse_port = true);
  ~Acceptor();

  /// @brief 设置新连接回调函数
  /// @param cb 新连接回调函数
  void setNewConnectionCallback(const NewConnectionCallback &cb) {
    cb_ = cb;
  }

  /// @brief 当前Acceptor是否处理监听状态
  /// @return 是否处于监听状态
  bool listening() const {
    return listening_;
  }

  /// @brief 开启监听，向poller注册读事件
  void Listen();

private:
  void HandleRead();

private:
  EventLoop *loop_;
  Socket accept_socket_;
  Channel accept_channel_;
  NewConnectionCallback cb_;
  bool listening_;
};
NAMESPACE_END
