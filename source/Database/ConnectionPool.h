/*
 * @Author: chenjingyu
 * @Date: 2024-09-18 17:42:17
 * @Contact: 2458006466@qq.com
 * @Description: SQLConnectionPool
 */
#pragma once

#include "Api.h"
#include <mysql/mysql.h>
#include <list>

#include "Base/Locker.h"

NAMESPACE_BEGIN
class ConnectionPool {
public:
  MYSQL *getConnection();
  bool ReleaseConnection(MYSQL *conn);
  int freeConn() const {
    return free_conn_;
  }

  void DestroyPool();
  static ConnectionPool *getInstance();
  void Init(
    const std::string &url,
    const std::string &user,
    const std::string &passwd,
    const std::string &dbname,
    unsigned int port,
    int max_conn
  );

private:
  ConnectionPool();
  ~ConnectionPool();

private:
  int max_conn_;
  int curr_conn_;
  int free_conn_;

  Locker locker_;
  std::list<MYSQL *> conn_list_;
  Sem reserve_;

  std::string url_;
  unsigned int port_;
  std::string user_;
  std::string passwd_;
  std::string dbname_;  
};

class ConnectionPollRAII {
public:
  ConnectionPollRAII(ConnectionPool *pool);
  ~ConnectionPollRAII();

  bool Update(const std::string &sql);
  MYSQL_RES *Query(const std::string &sql);

private:
  MYSQL *conn_raii;
  ConnectionPool *poll_raii;
};


NAMESPACE_END
