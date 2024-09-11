/*
 * @Author: chenjingyu
 * @Date: 2024-09-11 11:25:22
 * @Contact: 2458006466@qq.com
 * @Description: EventLoopThread
 */
#pragma once

#include "Api.h"
#include <mutex>
#include <condition_variable>

#include "Base/Thread.h"

NAMESPACE_BEGIN
class EventLoop;
class API EventLoopThread {
public:
  using ThreadInitCallback = std::function<void(EventLoop*)>;
  EventLoopThread(const ThreadInitCallback &cb=ThreadInitCallback(), const std::string &name="");

  ~EventLoopThread();
  // 开启线程
  EventLoop *StartLoop();

private:
  /// @brief 线程绑定的函数
  void ThreadFunc();

private:
  EventLoop *loop_;
  bool exiting_;
  Thread thread_;
  std::mutex mtx_;
  std::condition_variable cv_;
  ThreadInitCallback cb_;
};
NAMESPACE_END
