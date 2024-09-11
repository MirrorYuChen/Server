/*
 * @Author: chenjingyu
 * @Date: 2024-09-11 11:39:45
 * @Contact: 2458006466@qq.com
 * @Description: EventLoopThreadPool
 */
#pragma once

#include "Api.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>

NAMESPACE_BEGIN
class EventLoop;
class EventLoopThread;
/// @brief one loop per thread
class API EventLoopThreadPool {
public:
  using ThreadInitCallback = std::function<void(EventLoop*)>;
  EventLoopThreadPool(EventLoop *base_loop, const std::string &name);
  ~EventLoopThreadPool();

  void setThreadNum(int num_threads) {
    num_threads_ = num_threads;
  }

  void Start(const ThreadInitCallback &cb=ThreadInitCallback());
  EventLoop *getNextLoop();

  std::vector<EventLoop*> getAllLoops();

  bool started() const {
    return started_;
  }

  const std::string name() const {
    return name_;
  }

private:
  EventLoop *base_loop_;
  std::string name_;
  bool started_;
  int num_threads_;
  size_t next_;
  std::vector<std::unique_ptr<EventLoopThread>> threads_;
  std::vector<EventLoop*> loops_;
};
NAMESPACE_END

