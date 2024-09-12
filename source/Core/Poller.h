/*
 * @Author: chenjingyu
 * @Date: 2024-09-10 18:09:31
 * @Contact: 2458006466@qq.com
 * @Description: Poller
 */
#pragma once

#include "Api.h"
#include "Base/Timestamp.h"

#include <vector>
#include <unordered_map>

NAMESPACE_BEGIN
class EventLoop;
class Channel;
/// @brief 事件分发器，相当于Demultiplex
class API Poller {
public:
  using ChannelList = std::vector<Channel*>;

  Poller(EventLoop *loop);
  virtual ~Poller() = default;

  /**
   * @brief 对应于epoll_wait
   * @param timeout_ms 超时时间
   * @param active_channels poller对象上发生事件
   * @return Timestamp 获取到poller对象上发生事件的时间戳
   */
  virtual Timestamp Poll(int timeout_ms, ChannelList *active_channels) = 0;
  
  /**
   * @brief 更新poller与channel之间关系
   * @param channel 当前要进行更新的channel
   */
  virtual void UpdateChannel(Channel *channel) = 0;

  /**
   * @brief 从poller中移除对于当前channel的绑定
   * @param channel 待移除的channel
   */
  virtual void RemoveChannel(Channel *channel) = 0;

  /// @brief 判断当前channel是否注册到poller中
  /// @param Channel 当前channel
  /// @return 是否注册到poller中
  bool hasChannel(Channel *Channel) const;

  /// @brief EventLoop通过该接口获取默认IO复用实现方式
  /// @param Poller所属的事件循环EventLoop
  /// @return 创建的Poller对象
  static Poller *newDefaultPoller(EventLoop *loop);

protected:
  using ChannelMap = std::unordered_map<int, Channel*>;
  ChannelMap channels_;

private:
  // 定义在poller所属事件循环EventLoop
  EventLoop *owner_loop_;
};
NAMESPACE_END
