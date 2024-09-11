/*
 * @Author: chenjingyu
 * @Date: 2024-09-10 18:09:08
 * @Contact: 2458006466@qq.com
 * @Description: EventLoop
 */
#include "Core/EventLoop.h"
#include "Base/Logger.h"
#include "Core/Poller.h"
#include "Core/Channel.h"

#include <unistd.h>
#include <sys/eventfd.h>
#include <fcntl.h>

NAMESPACE_BEGIN
__thread EventLoop *t_loop_in_this_thread = nullptr;
constexpr const int kPollTimeMs = 10000;

int CreateEvenfd() {
  int evfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  CHECK(evfd != -1) << "eventfd error: " << evfd;
  return evfd;
}

EventLoop::EventLoop() :
  looping_(false),
  quit_(false),
  calling_pending_functors_(false),
  thread_id_(CurrentThread::tid()),
  poller_(Poller::newDefaultPoller(this)),
  wakeup_fd_(CreateEvenfd()),
  wakeup_channel_(new Channel(this, wakeup_fd_)),
  curr_active_channel_(nullptr) {
  if (t_loop_in_this_thread) {
    LogCritical("Another EventLoop exists.");
  } else {
    t_loop_in_this_thread = this;
  }

  // 设置wakeup_fd的事件类型及发生事件的回调函数
  wakeup_channel_->setReadCallback(
    std::bind(&EventLoop::HandleRead, this)
  );
  wakeup_channel_->EnableReading();
}

EventLoop::~EventLoop() {
  // 1.移除掉所有感兴趣事件
  wakeup_channel_->DisableAll();
  // 2.将channel从EventLoop中删除
  wakeup_channel_->Remove();
  // 3.关闭wakeup_fd_
  close(wakeup_fd_);
  // 4.将EventLoop指针为空
  t_loop_in_this_thread = nullptr;
}

void EventLoop::Loop() {
  looping_ = true;
  quit_ = false;

  LogInfo("EventLoop start looping.");
  while (!quit_) {
    active_channels_.clear();
    poll_return_time_ = poller_->Poll(kPollTimeMs, &active_channels_);
    for (Channel *channel : active_channels_) {
      channel->HandleEvent(poll_return_time_);
    }
    DoPendingFunctors();
  }

  looping_ = false;
}

void EventLoop::Quit() {
  quit_ = true;
  if (isInLoopThread()) {
    Wakeup();
  }
}

void EventLoop::RunInLoop(Functor func) {
  if (isInLoopThread()) {
    func();
  } else {
    QueueInLoop(func);
  }
}

void EventLoop::QueueInLoop(Functor func) {
  {
    std::unique_lock<std::mutex> lock(mtx_);
    pending_functors_.emplace_back(func);
  }
  if (!isInLoopThread() || calling_pending_functors_) {
    // 唤醒loop所在线程
    Wakeup();
  }
}

void EventLoop::Wakeup() {
  uint64_t one = 1;
  ssize_t n = write(wakeup_fd_, &one, sizeof(one));
  if (n != sizeof(one)) {
    LogError("EventLoop::Wakeup() writes {} bytes instead of 8.", n);
  }
}

void EventLoop::HandleRead() {
  uint64_t one = 1;
  ssize_t n = read(wakeup_fd_, &one, sizeof(one));
  if (n != sizeof(one)) {
    LogError("EventLoop::HanldeRead() reads {} bytes instead of 8.", n);
  }
}

void EventLoop::UpdateChannel(Channel *channel) {
  poller_->UpdateChannel(channel);
}

void EventLoop::RemoveChannel(Channel *channel) {
  poller_->RemoveChannel(channel);
}

bool EventLoop::hasChannel(Channel *Channel) {
  return poller_->hasChannel(Channel);
}

void EventLoop::DoPendingFunctors() {
  std::vector<Functor> functors;
  calling_pending_functors_ = true;
  {
    std::unique_lock<std::mutex>lock(mtx_);
    functors.swap(pending_functors_);
  }
  for (const auto &functor : functors) {
    functor();
  }
  calling_pending_functors_ = false;
}

NAMESPACE_END
