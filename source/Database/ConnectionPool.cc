/*
 * @Author: chenjingyu
 * @Date: 2024-09-18 17:42:26
 * @Contact: 2458006466@qq.com
 * @Description: ConnectionPool
 */
#include "Database/ConnectionPool.h"
#include "Base/Logger.h"

NAMESPACE_BEGIN
ConnectionPool::ConnectionPool() : curr_conn_(0), free_conn_(0) {

}

ConnectionPool::~ConnectionPool() {
  DestroyPool();
}

ConnectionPool *ConnectionPool::getInstance() {
  static ConnectionPool instance;
  return &instance;
}

void ConnectionPool::Init(
  const std::string &url,
  const std::string &user,
  const std::string &passwd,
  const std::string &dbname,
  unsigned int port,
  int max_conn
) {
  url_ = url;
  port_ = port;
  user_ = user;
  passwd_ = passwd;
  dbname_ = dbname;

  for (int i = 0; i < max_conn; ++i) {
    MYSQL *conn = NULL;
    conn = mysql_init(conn);
    if (conn == NULL) {
      LogError("init mysql failed.");
      exit(1);
    }
    conn = mysql_real_connect(
      conn, url_.c_str(), user_.c_str(), passwd_.c_str(),
      dbname_.c_str(), port_, NULL, 0
    );
    if (conn == NULL) {
      LogError("connect mysql failed!");
      exit(1);
    } else {
      // 设置中文数据集，C和C++代码默认的编码字符是ASCII，若不设置，中文都会乱码
      mysql_query(conn, "set names gbk");
      LogInfo("connect mysql succeed!");
    }
    conn_list_.push_back(conn);
    ++free_conn_;
  }

  reserve_ = Sem(free_conn_);
  max_conn_ = free_conn_;
}

MYSQL *ConnectionPool::getConnection() {
  MYSQL *conn = NULL;
  if (0 == conn_list_.size()) {
    return NULL;
  }
  reserve_.Wait();

  locker_.Lock();
  conn = conn_list_.front();
  conn_list_.pop_front();

  --free_conn_;
  ++curr_conn_;
  locker_.Unlock();
  return conn;
}

bool ConnectionPool::ReleaseConnection(MYSQL *conn) {
  if (NULL == conn) {
    return false;
  }

  locker_.Lock();
  conn_list_.push_back(conn);
  ++free_conn_;
  --curr_conn_;
  locker_.Unlock();

  reserve_.Post();

  return true;
}

void ConnectionPool::DestroyPool() {
  locker_.Lock();
  if (conn_list_.size() > 0) {
    for (auto &item : conn_list_) {
      mysql_close(item);
    }
  }
  curr_conn_ = 0;
  free_conn_ = 0;
  locker_.Unlock();
}

ConnectionPoolRAII::ConnectionPoolRAII(ConnectionPool *pool) {
  conn_raii = pool->getConnection();
  poll_raii = pool;
}

ConnectionPoolRAII::~ConnectionPoolRAII() {
  poll_raii->ReleaseConnection(conn_raii);
}

bool ConnectionPoolRAII::Execute(const std::string &sql) {
  if (mysql_query(conn_raii, sql.c_str()) != 0) {
    LogError("Failed Execute {}, Error is: {}.", sql, mysql_error(conn_raii));
    return false;
  }
  return true;
}

MYSQL_RES *ConnectionPoolRAII::Query(const std::string &sql) {
  if (!Execute(sql)) {
    LogError("Failed Query {}, Error is: {}.", sql, mysql_error(conn_raii));
    return nullptr;
  }
  return mysql_use_result(conn_raii);
}

NAMESPACE_END
