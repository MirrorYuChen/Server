/*
 * @Author: chenjingyu
 * @Date: 2024-09-11 16:30:56
 * @Contact: 2458006466@qq.com
 * @Description: TcpServer
 */
#pragma once

#include "Api.h"
#include <functional>
#include <string>
#include <memory>
#include <unordered_map>
#include <atomic>

#include "Core/EventLoop.h"
#include "Core/EventLoopThreadPool.h"
#include "Core/Acceptor.h"
#include "Core/InetAddress.h"
#include "Core/Callbacks.h"
#include "Core/TcpConnection.h"

NAMESPACE_BEGIN
class API TcpServer {
public:
  using ThreadInitCallback = std::function<void(EventLoop*)>;
  enum Option {
    kNoReusePort,
    kReusePort,
  };

  TcpServer(
    EventLoop *loop, 
    const InetAddress &listen_addr, 
    const std::string &name, 
    Option opt = kNoReusePort
  );
  ~TcpServer();

  void setThreadInitCallback(const ThreadInitCallback &cb) {
    thread_init_cb_ = cb;
  }

  void setConnectionCallback(const ConnectionCallback &cb) {
    conn_cb_ = cb;
  }

  void setMessageCallback(const MessageCallback &cb) {
    msg_cb_ = cb;
  }

  void setWriteCompleteCallback(const WriteCompleteCallback &cb) {
    wrt_cb_ = cb;
  }

  void setThreadNum(int num_thread);
  void Start();


  EventLoop *getLoop() const {
    return loop_;
  }

  const std::string &name() const {
    return name_;
  }

  const std::string &ipPort() const {
    return ip_port_;
  }

private:
  void NewConnection(int sockfd, const InetAddress &peer_addr);
  void RemoveConnection(const TcpConnectionPtr &conn_ptr);
  void RemoveConnectionInLoop(const TcpConnectionPtr &conn_ptr);

  NOT_ALLOWED_COPY(TcpServer)

private:
  using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;
  EventLoop *loop_;
  const std::string ip_port_;
  const std::string name_;

  std::unique_ptr<Acceptor> acceptor_;
  std::shared_ptr<EventLoopThreadPool> thread_pool_;

  ConnectionCallback conn_cb_;
  MessageCallback msg_cb_;
  WriteCompleteCallback wrt_cb_;

  ThreadInitCallback thread_init_cb_;
  std::atomic_int started_;

  int next_conn_id_;
  ConnectionMap conns_;
};

NAMESPACE_END
