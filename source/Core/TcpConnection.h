/*
 * @Author: chenjingyu
 * @Date: 2024-09-11 14:46:40
 * @Contact: 2458006466@qq.com
 * @Description: TcpConnection
 */
#pragma once

#include "Api.h"
#include <memory>
#include <atomic>
#include <string>

#include "Base/Timestamp.h"
#include "Base/Buffer.h"
#include "Core/InetAddress.h"
#include "Core/Callbacks.h"

NAMESPACE_BEGIN
class Channel;
class EventLoop;
class Socket;
/**
 * @brief 用于subLoop中，对连接的sockfd及其相关方法进行封装
 * (读时间、发送事件、消息事件、连接关闭事件及错误事件等)
 */
class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
  TcpConnection(EventLoop *loop, 
                const std::string &name, 
                int sockfd, 
                const InetAddress &local_addr, 
                const InetAddress &peer_addr);
  ~TcpConnection();

  EventLoop *getLoop() const {
    return loop_;
  }

  const std::string &name() const {
    return name_;
  }

  const InetAddress &localAddress() const {
    return local_addr_;
  }

  const InetAddress &peerAddress() const {
    return peer_addr_;
  }

  bool connected() const {
    return state_ == kConnected;
  }

  void Send(const std::string &buf);
  void Send(Buffer *buf);
  
  void Shutdown();

  void setConnectionCallback(const ConnectionCallback &cb) {
    conn_cb_ = cb;
  }

  void setMessageCallback(const MessageCallback &cb) {
    msg_cb_ = cb;
  }

  void setWriteCompleteCallback(const WriteCompleteCallback &cb) {
    wrt_cb_ = cb;
  }

  void setCloseCallback(const CloseCallback &cb) {
    cls_cb_ = cb;
  }

  void setHighWaterMarkCallback(const HighWaterMarkCallback &cb, size_t high_water_mark) {
    high_water_cb_ = cb;
    high_water_mark_ = high_water_mark;
  }

  void ConnectEstablished();
  void ConnectDestroyed();

private:
  enum StateEnum {
    kConnecting,      // 正在连接
    kConnected,       // 已连接
    kDisconnecting,   // 正在断开连接
    kDisconnected,    // 已断开连接
  };
  void setState(StateEnum state) {
    state_ = state;
  }

  /// @brief 读事件处理函数
  /// @param recv_time 获取监听事件的时间戳
  void HandleRead(Timestamp recv_time);

  /// @brief 写事件处理函数
  void HandleWrite();

  /// @brief 连接关闭处理函数
  void HandleClose();

  /// @brief 错误处理函数
  void HandleError();

  void SendInLoop(const std::string &str);
  void SendInLoopImpl(const void *msg, size_t len);
  void ShutdownInLoop();

  NOT_ALLOWED_COPY(TcpConnection)

private:
  /// @brief 轮询算法选取的subLoop
  EventLoop *loop_;
  const std::string name_;
  std::atomic_int state_;    // 连接状态
  bool reading_;

  /// @brief 保存已连接sockfd
  std::unique_ptr<Socket> socket_;
  /// @brief 封装上面socket_及各类事件处理函数
  std::unique_ptr<Channel> channel_;

  const InetAddress local_addr_;    // 服务器地址
  const InetAddress peer_addr_;     // 对端地址

  ConnectionCallback conn_cb_;
  MessageCallback msg_cb_;
  WriteCompleteCallback wrt_cb_;
  CloseCallback cls_cb_;
  HighWaterMarkCallback high_water_cb_;
  size_t high_water_mark_;

  /// @brief 对应Tcp连接用户接收缓冲区
  Buffer input_;
  /// @brief 对应Tcp连接发送缓冲区
  Buffer output_;
};

NAMESPACE_END
