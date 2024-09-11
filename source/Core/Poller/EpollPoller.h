/*
 * @Author: chenjingyu
 * @Date: 2024-09-11 10:30:23
 * @Contact: 2458006466@qq.com
 * @Description: EpollPoller
 */
#pragma once

#include "Api.h"
#include <sys/epoll.h>

#include "Core/Poller.h"
#include "Base/Timestamp.h"

NAMESPACE_BEGIN
class API EpollPoller : public Poller {
public:
  using EventList = std::vector<epoll_event>;
  EpollPoller(EventLoop *loop);
  ~EpollPoller() override;

  Timestamp Poll(int timeout_ms, ChannelList *active_channels) override;
  void UpdateChannel(Channel *channel) override;
  void RemoveChannel(Channel *Channel) override;

private:
  void FillActiveChannels(int num_evts, ChannelList *active_channels) const;
  void Update(int op, Channel *channel);

private:
  static constexpr const int kInitEventListSize = 16;
  int epfd_;
  EventList evts_;
};


NAMESPACE_END
