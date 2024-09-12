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
/**
 * @brief 封装了服务器监听sockfd及相关处理方法，内部其实没有核心处理函数，
 * 主要对其它类方法调用进行了封装。
 * (1) 服务器监听socket的文件描述符accept_socket_;
 * (2) 将accept_socket_、其感兴趣事件和对应处理函数封装的channel类accept_channel_;
 * (3) accept_socket_的sockfd由负责循环监听及处理相应事件的EventLoop对象loop_;
 * (4) TcpServer构造函数中将TcpServer::NewConnection()函数注册给cb_，公平选择
 * 一个SubLoop，并将已接收到的连接分发给这个subLoop；
 * (5) 将accept_channel_及其感兴趣事件注册到mainLoop上，让mainLoop监听accept_socket_;
 */
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
  /// @brief 接收新连接，并以负载均衡的方式选择一个subLoop
  /// 并将这个新连接分发到这个subLoop上
  void HandleRead();

private:
  /// @brief mainLoop
  EventLoop *loop_;
  /// @brief 接收到连接的socket
  Socket accept_socket_;
  /// @brief 接收到连接的channel
  Channel accept_channel_;
  /// @brief 创建新连接的回调函数
  NewConnectionCallback cb_;
  /// @brief 是否正在监听
  bool listening_;
};
NAMESPACE_END
