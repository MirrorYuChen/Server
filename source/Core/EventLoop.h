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
#include "Base/Logger.h"

NAMESPACE_BEGIN
class Channel;
class Poller;
/**
 * @brief 相当于Reactor，作为一个网络服务器，需要持续监听事件发生、持续获取监听结果和
 * 持续处理监听结果对应事件的能力，也就是需要去循环的调用Poller的Poll方法获取
 * 实际发生事件的channel集合，然后调用Channel中保存的事件处理函数处理相应事件。
 * EventLoop就是负责“循环”，及驱动“循环”的重要模块，Channel和Poller相当于
 * EventLoop的手下，EventLoop整合封装了二者并向上提供了更方便的接口使用。
 * Poller负责监听事件发生的结果，Channel则在其中起到将sockfd及相关属性封装的作用，
 * 将sockfd、其感兴趣事件、实际发生事件及不同事件对应回调函数封装在一起，这样各模块
 * 中传递更加方便。
 */
class API EventLoop {
public:
  using Functor = std::function<void()>;
  using ChannelList = std::vector<Channel*>;

  EventLoop();
  ~EventLoop();

  /// @brief 循环调用epoll_wait，不断获取发生事件的文件描述符
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
  /// @brief wakeup_fd_绑定读操作回调函数
  void HandleRead();
  /// @brief 执行上层回调
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

  /// @brief 绑定的poller
  std::unique_ptr<Poller> poller_;

  /// @brief mainLoop获取一个新用户Channel，需要轮询选择一个subLoop
  /// 通过该成员唤醒subLoop处理Channel
  int wakeup_fd_;
  std::unique_ptr<Channel> wakeup_channel_;

  /// @brief epoll_wait后获取到的事件
  ChannelList active_channels_;

  /// @brief 当前正在处理的Channel
  Channel *curr_active_channel_;

  /// @brief 保护线程安全执行回调函数
  std::mutex mtx_;

  /// @brief 存储Loop跨线程要执行的回调函数
  std::vector<Functor> pending_functors_;
};

static EventLoop *CheckLoopNotNull(EventLoop *loop) {
  CHECK(loop) << "mainLoop is nullptr!";
  return loop;
}

NAMESPACE_END
