/*
 * @Author: chenjingyu
 * @Date: 2024-09-10 18:09:18
 * @Contact: 2458006466@qq.com
 * @Description: Channel
 */
#pragma once

#include "Api.h"
#include <functional>
#include <memory>

NAMESPACE_BEGIN
class EventLoop;
class Timestamp;
/**
 * @brief Tcp网络编程中，若poller监听某个文件描述符，就要将这个sockfd和它
 * 感兴趣事件需要通过epoll_ctl注册到poller上。当poller监听到该sockfd上发生
 * 了某个事件，就会返回发生事件sockfd集合及每个sockfd发生的事件。
 * Channel类封装了以下内容：
 * (1) 这个Channel对象要照看的文件描述符sockfd_;
 * (2) sockfd_感兴趣事件events_;
 * (3) poller监听到sockfd_上发生的事件revents_;
 * (4) 这个Channel对象所属EventLoop对象loop_;
 * (5) 一些事件回调函数：读事件(read_cb_), 写事件(write_cb_), 关闭事件(close_cb_), 错误事件(error_cb_)
 */
class API Channel {
public:
  using EventCallback = std::function<void()>;
  using ReadEventCallback = std::function<void(Timestamp)>;

  Channel(EventLoop *loop, int sockfd);
  ~Channel();

  /**
   * @brief 调用epoll_wait后，可获知poller上哪些channel发生了事件，
   * 事件发生后，必然要调用这些channel对应的处理函数。HanleEvent让每个
   * 发生事件的channel根据实际发生的事件调用对应保存的事件处理函数
   * @param recv_time 获取事件发生的时间戳
   */
  void HandleEvent(Timestamp recv_time);

  /// @brief 设置一些事件处理的回调函数
  void setReadCallback(ReadEventCallback cb) {
    read_cb_ = std::move(cb);
  }

  void setWriteCallback(EventCallback cb) {
    write_cb_ = std::move(cb);
  }

  void setCloseCallback(EventCallback cb) {
    close_cb_ = std::move(cb);
  }

  void setErrorCallback(EventCallback cb) {
    error_cb_ = std::move(cb);
  }

  /// @brief TcpConnection建立时会调用
  /// @param obj 
  void Tie(const std::shared_ptr<void> &obj);

  /// @brief 返回封装的sockfd
  /// @return 封装的sockfd
  int sockfd() const {
    return sockfd_;
  }

  /// @brief 返回sockfd感兴趣事件
  /// @return sockfd感兴趣事件
  int events() const {
    return events_;
  }

  /// @brief 设置Poller上监听到的事件
  /// @param revents Poller上监听到的事件
  void setRevents(int revents) {
    revents_ = revents;
  }

  // 调用epoll_ctl向poller注册sockfd_感兴趣事件状态
  void EnableReading() {
    events_ |= kReadEvent;
    Update();
  }

  void DisableReading() {
    events_ &= ~kReadEvent;
    Update();
  }

  void EnableWriting() {
    events_ |= kWriteEvent;
    Update();
  }

  void DisableWriting() {
    events_ &= ~kWriteEvent;
    Update();
  }

  void DisableAll() {
    events_ &= kNoneEvent;
    Update();
  }

  bool isNoneEvent() const {
    return events_ == kNoneEvent;
  }

  bool isWriting() const {
    return events_ & kWriteEvent;
  }

  bool isReading() const {
    return events_ & kReadEvent;
  }

  /**
   * @brief 获取channel与poller相关状态
   * @return channel与poller相关状态：
   * kNew：还未被poller监听
   * kAdded：已被poller监听
   * kDeleted：已被移除
   */
  int index() const {
    return index_;
  }

  /// @brief 设置channel与poller相关状态
  /// @param index kNew(-1)，kAdded(1), kDeleted(2)
  void setIndex(int index) {
    index_ = index;
  }

  /// @brief 获取当前channel所属loop，one thread per loop
  /// @return 当前channel所属loop
  EventLoop *ownerLoop() const {
    return loop_;
  }

  /// @brief 从poller对象移除对当前channel监听
  void Remove();

private:
  /// @brief 调用epoll_ctl更新poller对象上相应感兴趣事件 
  /// EventLoop::UpdateChannel() => EpollPoller::UpdateChannel() 
  /// => epoll_ctl实现对sockfd_的增/删/改感兴趣事件
  void Update();
  /// @brief 根据事件执行channel中保存的回调函数
  /// @param recv_time poll所有IO就绪事件的时间戳
  void HandleEventWithGuard(Timestamp recv_time);

private:
  /**
   * const int Channel::kNoneEvent = 0;
   * const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
   * const int Channel::kWriteEvent = EPOLLOUT;
   */
  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  /// @brief 当前Channel所属EventLoop
  EventLoop *loop_;
  /// @brief 当前Channel管理在poller上监听对象
  const int sockfd_;
  /// @brief 注册sockfd感兴趣事件
  int events_;
  /// @brief 当前具体发生事件
  int revents_;
  /**
   * Poller上注册情况
   * const int kNew = -1;     // sockfd_还未被poller监视 
   * const int kAdded = 1;    // sockfd_正被poller监视中
   * const int kDeleted = 2;  // sockfd_被移除poller
   */ 
  int index_;

  // 弱指针指向TcpConnection
  // 必要时升级为shared_ptr，多一份引用计数，避免用户误删
  std::weak_ptr<void> tie_;
  // 标志此Channel是否被调用过Channel::Tie方法
  bool tied_;

  /// @brief 事件处理回调函数
  ReadEventCallback read_cb_;
  EventCallback write_cb_;
  EventCallback close_cb_;
  EventCallback error_cb_;
};

NAMESPACE_END
