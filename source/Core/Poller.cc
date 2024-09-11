/*
 * @Author: chenjingyu
 * @Date: 2024-09-10 18:09:36
 * @Contact: 2458006466@qq.com
 * @Description: Poller
 */
#include "Core/Poller.h"
#include "Core/Channel.h"

NAMESPACE_BEGIN
Poller::Poller(EventLoop *loop) : owner_loop_(loop) {}

bool Poller::hasChannel(Channel *channel) const {
  auto iter = channels_.find(channel->sockfd());
  return iter != channels_.end() && iter->second == channel;
}

NAMESPACE_END
