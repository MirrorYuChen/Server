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

void Channel::Tie(const std::shared_ptr<void> &obj) {
  tie_ = obj;
  tied_ = true;
}

void Channel::Update() {
  loop_->UpdateChannel(this);
}

void Channel::Remove() {
  loop_->RemoveChannel(this);
}

void Channel::HandleEvent(Timestamp recv_time) {
  if (tied_) {
    // 变成shared_ptr增加引用计数，防止误删
    std::shared_ptr<void> guard = tie_.lock();
    if (guard) {
      HandleEventWithGuard(recv_time);
    }
  } else {
    HandleEventWithGuard(recv_time);
  }
}

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
