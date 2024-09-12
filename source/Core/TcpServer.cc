/*
 * @Author: chenjingyu
 * @Date: 2024-09-11 16:31:02
 * @Contact: 2458006466@qq.com
 * @Description: TcpServer
 */
#include "Core/TcpServer.h"

NAMESPACE_BEGIN
TcpServer::TcpServer(
  EventLoop *loop, const InetAddress &listen_addr, 
  const std::string &name, Option opt) : 
  loop_(CheckLoopNotNull(loop)),
  ip_port_(listen_addr.toIpPort()),
  name_(name),
  acceptor_(new Acceptor(loop, listen_addr, opt == kReusePort)),
  thread_pool_(new EventLoopThreadPool(loop, name)),
  conn_cb_(),
  msg_cb_(),
  wrt_cb_(),
  thread_init_cb_(),
  started_(0),
  next_conn_id_(1) {
  acceptor_->setNewConnectionCallback(
    std::bind(&TcpServer::NewConnection, this, std::placeholders::_1, std::placeholders::_2)
  );
}

TcpServer::~TcpServer() {
  for (auto &item : conns_) {
    TcpConnectionPtr conn(item.second);
    item.second.reset();
    conn->getLoop()->RunInLoop(
      std::bind(&TcpConnection::ConnectDestroyed, conn)
    );
  }
}

void TcpServer::setThreadNum(int num_thread) {
  thread_pool_->setThreadNum(num_thread);
}

void TcpServer::Start() {
  if (started_++ == 0) {
    thread_pool_->Start(thread_init_cb_);
    loop_->RunInLoop(
      std::bind(&Acceptor::Listen, acceptor_.get())
    );
  }
}

void TcpServer::NewConnection(int sockfd, const InetAddress &peer_addr) {
  // 1.轮询算法选择一个subLoop管理connfd对应channel
  EventLoop *io_loop = thread_pool_->getNextLoop();
  std::string conn_name = ip_port_ + "#" + std::to_string(next_conn_id_++);
  LogInfo("TcpServer::NewConnection [{}] - new connection [{}] from {}", 
    name_, conn_name, peer_addr.toIpPort()
  );

  // 2.获取sockfd对应socket地址
  sockaddr_in local;
  memset(&local, 0, sizeof(local));
  socklen_t addr_len = sizeof(local);
  if (getsockname(sockfd, (sockaddr*)&local, &addr_len) < 0) {
    LogError("getsockname() failed.");
  }

  // 3.构建TcpConnectionPtr，并进行存储
  InetAddress local_addr(local);
  TcpConnectionPtr conn_ptr(
    new TcpConnection(
      io_loop,
      conn_name,
      sockfd,
      local_addr,
      peer_addr
    )
  );
  conns_[conn_name] = conn_ptr;

  // 4.设置回调
  conn_ptr->setConnectionCallback(conn_cb_);
  conn_ptr->setMessageCallback(msg_cb_);
  conn_ptr->setWriteCompleteCallback(wrt_cb_);
  conn_ptr->setCloseCallback(
    std::bind(&TcpServer::RemoveConnection, this, std::placeholders::_1)
  );

  // 5.调用创建连接
  io_loop->RunInLoop(
    std::bind(&TcpConnection::ConnectEstablished, conn_ptr)
  );
}

void TcpServer::RemoveConnection(const TcpConnectionPtr &conn_ptr) {
  loop_->RunInLoop(
    std::bind(&TcpServer::RemoveConnectionInLoop, this, conn_ptr)
  );
}

void TcpServer::RemoveConnectionInLoop(const TcpConnectionPtr &conn_ptr) {
  LogInfo("TcpServer::RemoveConnectionInLoop() [{}] - connection [{}]", name_, conn_ptr->name());
  conns_.erase(conn_ptr->name());
  EventLoop *io_loop = conn_ptr->getLoop();
  io_loop->QueueInLoop(
    std::bind(&TcpConnection::ConnectDestroyed, conn_ptr)
  );
}

NAMESPACE_END
