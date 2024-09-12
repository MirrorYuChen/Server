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

/// @brief 创建一个可用于事件通知的文件描述符
/// @return 文件描述符
int CreateEvenfd() {
  int evfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  CHECK(evfd != -1) << "eventfd error: " << evfd;
  return evfd;
}

/// @brief EventLoop构造函数，每个EventLoop对象都唯一绑定了一个线程
EventLoop::EventLoop() :
  looping_(false),
  quit_(false),
  calling_pending_functors_(false),
  thread_id_(CurrentThread::tid()),
  poller_(Poller::newDefaultPoller(this)),
  wakeup_fd_(CreateEvenfd()),
  wakeup_channel_(new Channel(this, wakeup_fd_)),
  curr_active_channel_(nullptr) {
  // 该线程已绑定某个EventLoop对象了，那么该线程就无法创建新EventLoop对象
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

/// @brief 开启事件循环
void EventLoop::Loop() {
  looping_ = true;
  quit_ = false;

  LogInfo("EventLoop start looping.");
  while (!quit_) {
    // 1.epoll_wait获取poller对象上发生的事件
    active_channels_.clear();
    poll_return_time_ = poller_->Poll(kPollTimeMs, &active_channels_);
    // 2.执行对应事件处理函数
    for (Channel *channel : active_channels_) {
      channel->HandleEvent(poll_return_time_);
    }
    // 3.处理跨线程调用的回调函数
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

/// @brief 在loop中执行任务函数
/// @param func 任务函数
void EventLoop::RunInLoop(Functor func) {
  if (isInLoopThread()) {
    func();
  } else {
    QueueInLoop(func);
  }
}

/// @brief 将任务添加到队列中
/// @param func 任务函数
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

/// @brief 唤醒其它线程(one thread per loop)，也就是唤醒subLoop
void EventLoop::Wakeup() {
  uint64_t one = 1;
  ssize_t n = write(wakeup_fd_, &one, sizeof(one));
  if (n != sizeof(one)) {
    LogError("EventLoop::Wakeup() writes {} bytes instead of 8.", n);
  }
}

/// @brief wakeup_channel_绑定的读回调函数
void EventLoop::HandleRead() {
  uint64_t one = 1;
  ssize_t n = read(wakeup_fd_, &one, sizeof(one));
  if (n != sizeof(one)) {
    LogError("EventLoop::HanldeRead() reads {} bytes instead of 8.", n);
  }
}

/// @brief poller更新channel关注的事件
/// @param channel 待更新的channel
void EventLoop::UpdateChannel(Channel *channel) {
  poller_->UpdateChannel(channel);
}

/// @brief 从poller中删除当前channel
/// @param channel 待删除channel
void EventLoop::RemoveChannel(Channel *channel) {
  poller_->RemoveChannel(channel);
}

/// @brief poller对象中是否注册了当前channel
/// @param Channel 待查询的当前channel
/// @return 注册与否
bool EventLoop::hasChannel(Channel *Channel) {
  return poller_->hasChannel(Channel);
}

/// @brief 执行注册的回调函数
void EventLoop::DoPendingFunctors() {
  // 1.获取所有回调函数
  std::vector<Functor> functors;
  calling_pending_functors_ = true;
  {
    std::unique_lock<std::mutex>lock(mtx_);
    functors.swap(pending_functors_);
  }
  // 2.执行所有回调函数
  for (const auto &functor : functors) {
    functor();
  }
  // 3.更新相关标志位
  calling_pending_functors_ = false;
}

NAMESPACE_END
