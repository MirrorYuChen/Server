/*
 * @Author: chenjingyu
 * @Date: 2024-09-11 11:39:57
 * @Contact: 2458006466@qq.com
 * @Description: EventLoopThreadPool
 */
#include "Core/EventLoopThreadPool.h"
#include <memory>
#include "Core/EventLoopThread.h"

NAMESPACE_BEGIN
EventLoopThreadPool::EventLoopThreadPool(EventLoop *base_loop, const std::string &name) :
  base_loop_(base_loop),
  name_(name),
  started_(false),
  num_threads_(0),
  next_(0) {
 
}

EventLoopThreadPool::~EventLoopThreadPool() = default;

void EventLoopThreadPool::Start(const ThreadInitCallback &cb) {
  started_ = true;
  for (int i = 0; i < num_threads_; ++i) {
    EventLoopThread *t = new EventLoopThread(cb, name_ + std::to_string(i));
    threads_.push_back(std::unique_ptr<EventLoopThread>(t));
    loops_.push_back(t->StartLoop());
  }
  if (num_threads_ == 0 && cb) {
    cb(base_loop_);
  }
}

EventLoop *EventLoopThreadPool::getNextLoop() {
  // 1.单线程只有mainReactor，无subReactor，使用base_loop_
  EventLoop *loop = base_loop_;

  // 2.多线程轮询方式获取下一个处理事件的loop
  if (!loops_.empty()) {
    loop = loops_[next_++];
    if (next_ >= loops_.size()) {
      next_ = 0;
    }
  }
  return loop;
}

std::vector<EventLoop *> EventLoopThreadPool::getAllLoops() {
  if (loops_.empty()) {
    return std::vector<EventLoop *>(1, base_loop_);
  } else {
    return loops_;
  }
}


NAMESPACE_END
