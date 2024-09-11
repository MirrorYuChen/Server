/*
 * @Author: chenjingyu
 * @Date: 2024-09-11 11:02:24
 * @Contact: 2458006466@qq.com
 * @Description: DefaultPoller
 */
#include "Core/Poller.h"
#include "Core/Poller/EpollPoller.h"

NAMESPACE_BEGIN
Poller *Poller::newDefaultPoller(EventLoop *loop) {
  return new EpollPoller(loop);
}

NAMESPACE_END
