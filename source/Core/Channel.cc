/*
 * @Author: chenjingyu
 * @Date: 2024-09-10 18:09:22
 * @Contact: 2458006466@qq.com
 * @Description: Channel
 */
#include "Core/Channel.h"

#include <sys/epoll.h>

#include "Core/EventLoop.h"
#include "Base/Timestamp.h"
#include "Base/Logger.h"

NAMESPACE_BEGIN
const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop *loop, int sockfd) :
  loop_(loop),
  sockfd_(sockfd),
  events_(0),
  revents_(0),
  index_(-1),
  tied_(false) {

}

Channel::~Channel() = default;

/// @brief TcpConnection建立时调用
/// @param obj 建立到TcpConnectionPtr的绑定
void Channel::Tie(const std::shared_ptr<void> &obj) {
  // 1.weak_ptr指向TcpConnectionPtr
  tie_ = obj;
  // 2.设置绑定标志
  tied_ = true;
}

/// @brief 更新poller对象上channel关注事件
void Channel::Update() {
  loop_->UpdateChannel(this);
}

/// @brief 从poller对象上移除当前channel
void Channel::Remove() {
  loop_->RemoveChannel(this);
}

/// @brief 从poller对象上获取事件后，进行处理
/// @param recv_time 获取事件的时间戳
void Channel::HandleEvent(Timestamp recv_time) {
  if (tied_) {
    // 判断TcpConnectionPtr是否还存在，存在才需要处理，不存在就不处理
    std::shared_ptr<void> guard = tie_.lock();
    if (guard) {
      HandleEventWithGuard(recv_time);
    }
  } else {
    HandleEventWithGuard(recv_time);
  }
}

/// @brief 具体事件处理方法
/// @param recv_time 获取事件的时间戳
void Channel::HandleEventWithGuard(Timestamp recv_time) {
  // 1.对端关闭事件
  if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
    if (close_cb_) {
      close_cb_();
    }
  }

  // 2.错误事件
  if (revents_ & EPOLLERR) {
    LogError("sockfd: {}", sockfd());
    if (error_cb_) {
      error_cb_();
    }
  }

  // 3.读事件
  if (revents_ & (EPOLLIN | EPOLLPRI)) {
    LogInfo("read callback fd: {}.", sockfd());
    if (read_cb_) {
      read_cb_(recv_time);
    }
  }

  // 4.写事件
  if (revents_ & EPOLLOUT) {
    if (write_cb_) {
      write_cb_();
    }
  }
}

NAMESPACE_END
