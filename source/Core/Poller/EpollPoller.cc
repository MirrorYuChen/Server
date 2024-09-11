/*
 * @Author: chenjingyu
 * @Date: 2024-09-11 10:30:40
 * @Contact: 2458006466@qq.com
 * @Description: EpollPoller
 */
#include "Core/Poller/EpollPoller.h"
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "Base/Logger.h"
#include "Core/Channel.h"

NAMESPACE_BEGIN
// channel未添加到poller
constexpr const int kNew = -1; 
// channel已添加到poller
constexpr const int kAdded = 1; 
// channel已从poller删除
constexpr const int kDeleted = 2; 

EpollPoller::EpollPoller(EventLoop *loop) :
  Poller(loop),
  epfd_(epoll_create1(EPOLL_CLOEXEC)),
  evts_(kInitEventListSize) {
  if (epfd_ < 0) {
    LogError("epoll_create() error: {}.", errno);
  }
}

EpollPoller::~EpollPoller() {
  close(epfd_);
}

Timestamp EpollPoller::Poll(int timeout_ms, ChannelList *active_channels) {
  size_t num_evts = epoll_wait(epfd_, evts_.data(), static_cast<int>(evts_.size()), timeout_ms);
  int save_errno = errno;
  Timestamp now(Timestamp::Now());
  if (num_evts > 0) {
    FillActiveChannels(num_evts, active_channels);
    // 扩容操作
    if (num_evts == evts_.size()) {
      evts_.resize(evts_.size() * 2);
    }
  } else if (num_evts == 0) {
    LogDebug("timeout!");
  } else {
    // 不是终端错误
    if (save_errno != EINTR) {
      errno = save_errno;
      LogError("EpollPoller::Poll() failed.");
    }
  }
  return now;
}

void EpollPoller::UpdateChannel(Channel *channel) {
  const int index = channel->index();
  if (index == kNew || index == kDeleted) {
    if (index == kNew) {
      int sockfd = channel->sockfd();
      channels_[sockfd] = channel;
    }
    channel->setIndex(kAdded);
    Update(EPOLL_CTL_ADD, channel);
  } else {
    if (channel->isNoneEvent()) {
      Update(EPOLL_CTL_DEL, channel);
      channel->setIndex(kDeleted);
    } else {
      Update(EPOLL_CTL_MOD, channel);
    }
  }
}

void EpollPoller::FillActiveChannels(int num_evts, ChannelList *active_channels) const {
  for (int i = 0; i < num_evts; ++i) {
    Channel *channel = static_cast<Channel*>(evts_[i].data.ptr);
    channel->setRevents(evts_[i].events);
    active_channels->push_back(channel);
  }
}

void EpollPoller::RemoveChannel(Channel *channel) {
  int sockfd = channel->sockfd();
  channels_.erase(sockfd);

  int idx = channel->index();
  if (idx == kAdded) {
    Update(EPOLL_CTL_DEL, channel);
  }
  channel->setIndex(kNew);
}

void EpollPoller::Update(int op, Channel *channel) {
  struct epoll_event ev;
  memset(&ev, 0, sizeof(ev));

  int sockfd = channel->sockfd();
  ev.events = channel->events();
  ev.data.fd = sockfd;
  ev.data.ptr = channel;

  if (epoll_ctl(epfd_, op, sockfd, &ev) < 0) {
    if (op == EPOLL_CTL_DEL) {
      LogError("epoll_ctl() del error: {}.", errno);
    } else {
      LogCritical("epoll_ctl() add/mod error: {}.", errno);
    }
  }
}


NAMESPACE_END
