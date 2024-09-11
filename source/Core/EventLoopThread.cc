/*
 * @Author: chenjingyu
 * @Date: 2024-09-11 11:25:29
 * @Contact: 2458006466@qq.com
 * @Description: EventLoopThread
 */
#include "Core/EventLoopThread.h"
#include "Core/EventLoop.h"

NAMESPACE_BEGIN
EventLoopThread::EventLoopThread(const ThreadInitCallback &cb, const std::string &name) :
  loop_(nullptr),
  exiting_(false),
  thread_(std::bind(&EventLoopThread::ThreadFunc, this), name),
  mtx_(),
  cv_(),
  cb_(cb) {
  
}

EventLoopThread::~EventLoopThread() {
  exiting_ = true;
  if (loop_) {
    loop_->Loop();
    thread_.Join();
  }
}

// 开启线程
EventLoop *EventLoopThread::StartLoop() {
  thread_.Start();
  EventLoop *loop = nullptr;
  {
    std::unique_lock<std::mutex> lock(mtx_);
    while (!loop_) {
      cv_.wait(lock);
    }
    loop = loop_;
  }
  return loop;
}

void EventLoopThread::ThreadFunc() {
  EventLoop loop;
  if (cb_) {
    cb_(&loop);
  }
  {
    std::unique_lock<std::mutex> lock(mtx_);
    loop_ = &loop;
    cv_.notify_one();
  }

  // 这个是subLoop
  loop.Loop();
  std::unique_lock<std::mutex> lock(mtx_);
  loop_ = nullptr;
}

NAMESPACE_END
