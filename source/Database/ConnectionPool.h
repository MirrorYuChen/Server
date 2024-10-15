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
  /// @brief 单例模式，获取连接池单例
  /// @return 连接池单例
  static ConnectionPool *getInstance();

  /// @brief 获取一条连接
  /// @return 一条连接
  MYSQL *getConnection();

  /// @brief 用完连接后，将连接存回连接池中
  /// @param conn 当前连接
  /// @return 当前连接是否为nullptr
  bool ReleaseConnection(MYSQL *conn);

  /// @brief 返回当前空闲连接数
  /// @return 当前空闲连接数
  int freeConn() const {
    return free_conn_;
  }

  /// @brief 销毁连接池
  void DestroyPool();

  /// @brief 初始化连接池
  /// @param url 主机地址 
  /// @param user 登录用户名
  /// @param passwd 登录用户密码
  /// @param dbname 使用数据库名
  /// @param port 数据库端口
  /// @param max_conn 最大连接数
  void Init(
    const std::string &url,
    const std::string &user,
    const std::string &passwd,
    const std::string &dbname,
    unsigned int port,
    int max_conn
  );

private:
  /// @brief 构造函数
  ConnectionPool();
  
  /// @brief 析构函数
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
class API ConnectionPoolRAII {
public:
  /// @brief 连接池RAII封装构造函数
  /// @param pool 注入的连接池
  ConnectionPoolRAII(ConnectionPool *pool);

  /// @brief 连接池RAII析构函数，利用对象生命周期，完成连接回收
  ~ConnectionPoolRAII();

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
