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
/// @brief SQL数据库连接池
class API ConnectionPool {
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
  /// @brief 池中最大连接数
  int max_conn_;

  /// @brief 当前已使用连接数
  int curr_conn_;

  /// @brief 当前空闲连接数
  int free_conn_;

  Locker locker_;

  /// @brief 连接池中连接列表
  std::list<MYSQL *> conn_list_;
  Sem reserve_;

  /// @brief 主机地址
  std::string url_;

  /// @brief 数据库端口号
  unsigned int port_;
  
  /// @brief 登录数据库用户名
  std::string user_;

  /// @brief 登录数据库用户密码
  std::string passwd_;

  /// @brief 使用数据库名
  std::string dbname_;  
};

/// @brief SQL连接池的RAII封装
class API ConnectionPollRAII {
public:
  /// @brief 连接池RAII封装构造函数
  /// @param pool 注入的连接池
  ConnectionPollRAII(ConnectionPool *pool);

  /// @brief 连接池RAII析构函数，利用对象生命周期，完成连接回收
  ~ConnectionPollRAII();

  /// @brief 执行操作
  /// @param sql 待执行的语句
  /// @return 是否执行成功
  bool Execute(const std::string &sql);

  /// @brief 查询
  /// @param sql 查询语句
  /// @return 查询结果
  MYSQL_RES *Query(const std::string &sql);

private:
  /// @brief 当前从连接池中获取到的连接
  MYSQL *conn_raii;

  /// @brief 当前传入的连接池
  ConnectionPool *poll_raii;
};


NAMESPACE_END
