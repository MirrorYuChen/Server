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
  // 1.获取poller与channel之间相关状态
  const int index = channel->index();
  // 2.添加channel，及监听事件
  if (index == kNew || index == kDeleted) {
    if (index == kNew) {
      int sockfd = channel->sockfd();
      channels_[sockfd] = channel;
    }
    channel->setIndex(kAdded);
    Update(EPOLL_CTL_ADD, channel);
  } else {
    // 2.删除channel监听事件
    if (channel->isNoneEvent()) {
      Update(EPOLL_CTL_DEL, channel);
      channel->setIndex(kDeleted);
    } else {
      // 3.修改channel监听事件
      Update(EPOLL_CTL_MOD, channel);
    }
  }
}

/// @brief 将epoll_wait监听到事件打包成ChannelList
/// @param num_evts 监听到事件数目
/// @param active_channels 打包后的Channel列表
void EpollPoller::FillActiveChannels(int num_evts, ChannelList *active_channels) const {
  for (int i = 0; i < num_evts; ++i) {
    Channel *channel = static_cast<Channel*>(evts_[i].data.ptr);
    channel->setRevents(evts_[i].events);
    active_channels->push_back(channel);
  }
}

/// @brief 从poller中移除对channel的绑定
/// @param channel 待处理channel 
void EpollPoller::RemoveChannel(Channel *channel) {
  // 1.从列表中删除当前channel
  int sockfd = channel->sockfd();
  channels_.erase(sockfd);

  // 2.更新poller上当前channel的关注事件
  int idx = channel->index();
  if (idx == kAdded) {
    Update(EPOLL_CTL_DEL, channel);
  }

  // 3.设置channel与poller之间关联
  channel->setIndex(kNew);
}

/// @brief 更新poller上当前channel监听事件
/// @param op 当前channel监听事件
/// @param channel 当前channel
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
