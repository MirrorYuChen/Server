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
/// @brief 对文件描述符和感兴趣事件的封装
class API Channel {
public:
  using EventCallback = std::function<void()>;
  using ReadEventCallback = std::function<void(Timestamp)>;

  Channel(EventLoop *loop, int sockfd);
  ~Channel();

  void HandleEvent(Timestamp recv_time);

  /// 设置一些事件处理的回调函数
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

  /// @brief 设置Poll时发生事件
  /// @param revents Poll时发生事件
  void setRevents(int revents) {
    revents_ = revents;
  }

  // 调用epoll_ctl来设置sockfd相应事件状态
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
