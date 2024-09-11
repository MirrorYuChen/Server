/*
 * @Author: chenjingyu
 * @Date: 2024-09-10 18:09:03
 * @Contact: 2458006466@qq.com
 * @Description: EventLoop
 */
#pragma once

#include "Api.h"
#include <atomic>
#include <vector>
#include <functional>
#include <memory>
#include <mutex>

#include "Base/Timestamp.h"
#include "Base/CurrentThread.h"

NAMESPACE_BEGIN
class Channel;
class Poller;
/// @brief 事件循环：包含Channel和Poller
class API EventLoop {
public:
  using Functor = std::function<void()>;
  using ChannelList = std::vector<Channel*>;

  EventLoop();
  ~EventLoop();

  void Loop();
  void Quit();

  Timestamp pollReturnTime() const {
    return poll_return_time_;
  }

  /// @brief 在当前线程同步调用函数
  /// @param func 调用的函数
  void RunInLoop(Functor func);

  /// @brief 将调用函数存入队列，唤醒Loop所在线程执行func
  /// @param func 调用函数
  void QueueInLoop(Functor func);

  /// @brief 唤醒loop所在线程
  void Wakeup();

  /// EventLoop => Poller
  void UpdateChannel(Channel *channel);
  void RemoveChannel(Channel *channel);
  bool hasChannel(Channel *channel);

  /// @brief 判断EventLoop是否在自己线程
  /// @return 是否在自己所在线程
  bool isInLoopThread() const {
    return thread_id_ == CurrentThread::tid();
  }

private:
  void HandleRead();
  void DoPendingFunctors();

private:
  /// @brief 标志当前是否在进行事件循环, 
  /// Loop接口调用时，起始阶段looping_置为true
  /// 结束阶段置为false
  std::atomic_bool looping_;

  /// @brief 标志是否退出Loop循环
  std::atomic_bool quit_;

  /// @brief 标志当前Loop是否有要执行的回调函数
  std::atomic_bool calling_pending_functors_;

  /// @brief 记录当前Loop所在线程id
  const pid_t thread_id_;

  /// @brief Poller返回发生事件Channels的返回时间
  Timestamp poll_return_time_;

  /// @brief 当前绑定的poller
  std::unique_ptr<Poller> poller_;

  /// @brief mainLoop获取一个新用户Channel，需要轮询选择一个subLoop
  /// 通过该成员唤醒subLoop处理Channel
  int wakeup_fd_;
  std::unique_ptr<Channel> wakeup_channel_;

  /// @brief Poll后IO就绪的Channel
  ChannelList active_channels_;

  /// @brief 当前正在处理的Channel
  Channel *curr_active_channel_;

  /// @brief 保护线程安全执行回调函数
  std::mutex mtx_;

  /// @brief 存储Loop跨线程要执行的回调函数
  std::vector<Functor> pending_functors_;
};

NAMESPACE_END
