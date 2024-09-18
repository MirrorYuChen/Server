/*
 * @Author: chenjingyu
 * @Date: 2024-09-11 14:46:47
 * @Contact: 2458006466@qq.com
 * @Description: TcpConnection
 */
#include "Core/TcpConnection.h"
#include <functional>
#include <string>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <functional>

#include "Base/Logger.h"
#include "Core/Socket.h"
#include "Core/Channel.h"
#include "Core/EventLoop.h"

NAMESPACE_BEGIN
TcpConnection::TcpConnection(
  EventLoop *loop, const std::string &name, 
  int sockfd, const InetAddress &local_addr, 
  const InetAddress &peer_addr) :
  loop_(CheckLoopNotNull(loop)),
  name_(name),
  state_(kConnecting),
  reading_(true),
  socket_(new Socket(sockfd)),
  channel_(new Channel(loop, sockfd)),
  local_addr_(local_addr),
  peer_addr_(peer_addr),
  high_water_mark_(64 * 1024 * 1024) {
  channel_->setReadCallback(
    std::bind(&TcpConnection::HandleRead, this, std::placeholders::_1)
  );
  channel_->setWriteCallback(
    std::bind(&TcpConnection::HandleWrite, this)
  );
  channel_->setCloseCallback(
    std::bind(&TcpConnection::HandleClose, this)
  );
  channel_->setErrorCallback(
    std::bind(&TcpConnection::HandleError, this)
  );

  LogInfo("TcpConnection::ctor [{}] at fd = {}", name_, sockfd);
  socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
  LogInfo("TcpConnection::dtor [{}] at fd = {}.", name_, channel_->sockfd());
}

void TcpConnection::Send(const std::string &buf) {
  if (state_ == kConnected) {
    if (loop_->isInLoopThread()) {
      SendInLoop(buf);
    } else {
      loop_->RunInLoop(
        std::bind(&TcpConnection::SendInLoop, this, buf)
      );
    }
  }
}

void TcpConnection::Send(Buffer *buf) {
  if (state_ == kConnected) {
    std::string str = buf->RetrieveAllAsString();
    if (loop_->isInLoopThread()) {
      SendInLoop(str);
    } else {
      loop_->RunInLoop(
        std::bind(&TcpConnection::SendInLoop, this, str)
      );
    }
  }
}

void TcpConnection::SendInLoopImpl(const void *msg, size_t len) {
  ssize_t nwrote = 0;
  size_t remaining = len;
  bool fault_error = false;

  // 1.前面调用了Shutdown，不能再发送
  if (state_ == kDisconnected) {
    LogError("Disconnected, give up writing.");
    return;
  }

  // 2.channel第一次写数据，且缓冲区无待发送数据
  if (!channel_->isWriting() && output_.ReadableBytes() == 0) {
    nwrote = write(channel_->sockfd(), msg, len);
    if (nwrote >= 0) {
      // 2.1 判断是否一次写完
      remaining = len - nwrote;
      if (remaining == 0 && wrt_cb_) {
        loop_->QueueInLoop(
          std::bind(wrt_cb_, shared_from_this())
        );
      }
    } else {
      nwrote = 0;
      if (errno != EWOULDBLOCK) {
        LogError("TcpConnection::SendInLoop.");
        if (errno == EPIPE || errno == ECONNRESET) {
          fault_error = true;
        }
      }
    }
  }

  if (fault_error || remaining <= 0) {
    return;
  }

  // 3.一次没发送完数据，剩余数据存入缓冲区，需改channel注册写事件
  size_t old_len = output_.ReadableBytes();
  if (old_len + remaining >= high_water_mark_ &&
      old_len < high_water_mark_ &&
      high_water_cb_ ) {
    loop_->QueueInLoop(
      std::bind(high_water_cb_, shared_from_this(), old_len + remaining)
    );
  }
  output_.Append((char*)msg + nwrote, remaining);
  if (!channel_->isWriting()) {
    // 注册channel的写事件
    channel_->EnableWriting();
  }
}

void TcpConnection::SendInLoop(const std::string &str) {
  SendInLoopImpl(str.c_str(), str.size());
}

void TcpConnection::Shutdown() {
  if (state_ == kConnected) {
    setState(kDisconnecting);
    loop_->RunInLoop(
      std::bind(&TcpConnection::ShutdownInLoop, this)
    );
  }
}

void TcpConnection::ShutdownInLoop() {
  // output_中数据已全部发送完成
  if (!channel_->isWriting()) {
    socket_->ShutdownWrite();
  }
}

/// @brief 创建连接
void TcpConnection::ConnectEstablished() {
  // 1.建立连接，设置状态为连接态
  setState(kConnected);
  // 2.让channel记录相关TcpConnection
  channel_->Tie(shared_from_this());
  // 3.向poller注册channel的EPOLLIN读事件
  channel_->EnableReading();
  // 4.调用连接回调函数
  conn_cb_(shared_from_this());
}

/// @brief 销毁连接
void TcpConnection::ConnectDestroyed() {
  if (state_ == kConnected) {
    setState(kDisconnected);
    channel_->DisableAll();
    conn_cb_(shared_from_this());
  }
  channel_->Remove();
}

void TcpConnection::HandleRead(Timestamp recv_time) {
  int save_errno = 0;
  // 1.从客户端socket读取数据到input_
  ssize_t n = input_.ReadFd(channel_->sockfd(), &save_errno);
  if (n > 0) {
    // 2.调用回调函数处理输入数据
    msg_cb_(shared_from_this(), &input_, recv_time);
  } else if (n == 0) {
    // 3.没有数据，客户端关闭连接
    HandleClose();
  } else {
    // 4.出现错误
    errno = save_errno;
    LogError("TcpConnection::HandleRead() Failed.");
    HandleError();
  }
}

void TcpConnection::HandleWrite() {
  if (channel_->isWriting()) {
    int save_errno = 0;
    // 1.将output_数据写入客户端socket
    ssize_t n = output_.WriteFd(channel_->sockfd(), &save_errno);
    if (n > 0) {
      // 2.更新内容索引
      output_.Retrieve(n);
      // 3.output_中数据读取完毕并写入到客户端socket
      if (output_.ReadableBytes() == 0) {
        channel_->DisableWriting();
        if (wrt_cb_) {
          loop_->QueueInLoop(
            std::bind(wrt_cb_, shared_from_this())
          );
        }
        if (state_ == kDisconnecting) {
          ShutdownInLoop();
        }
      }
    } else {
      LogError("TcpConnection::HandleWrite() failed.");
    }
  } else {
    // 不可写状态
    LogError("TcpConnection fd = {} is down, no more writing.", channel_->sockfd());
  }
}

void TcpConnection::HandleClose() {
  setState(kDisconnected);
  channel_->DisableAll();

  TcpConnectionPtr conn_ptr(shared_from_this());
  conn_cb_(conn_ptr);
  cls_cb_(conn_ptr);
}

void TcpConnection::HandleError() {
  int opt_val;
  socklen_t opt_len = sizeof(opt_val);
  int err = 0;
  if (getsockopt(channel_->sockfd(), SOL_SOCKET, SO_ERROR, &opt_val, &opt_len)) {
    err = errno;
  } else {
    err = opt_val;
  }
  LogError("TcpConnection::HandleError name: {} - Error: {}.", name_, err);
}


NAMESPACE_END
