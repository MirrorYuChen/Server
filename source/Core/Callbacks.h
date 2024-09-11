/*
 * @Author: chenjingyu
 * @Date: 2024-09-11 14:53:17
 * @Contact: 2458006466@qq.com
 * @Description: Callbacks
 */
#pragma once

#include "Api.h"
#include <memory>
#include <functional>

NAMESPACE_BEGIN
class Buffer;
class TcpConnection;
class Timestamp;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
using CloseCallback = std::function<void(const TcpConnectionPtr &)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;
using MessageCallback = std::function<void(const TcpConnectionPtr &, Buffer *, Timestamp)>;
using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr &, size_t)>;

NAMESPACE_END
