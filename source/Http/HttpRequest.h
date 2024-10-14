/*
 * @Author: chenjingyu
 * @Date: 2024-09-18 10:23:36
 * @Contact: 2458006466@qq.com
 * @Description: HttpRequest
 */
#pragma once

#include "Api.h"
#include "Base/Timestamp.h"
#include <unordered_map>

NAMESPACE_BEGIN
class API HttpRequest {
public:
  enum Method {
    kInvalid = -1,
    kGet = 0,
    kPost = 1,
    kHead = 2,
    kPut = 3,
    kDelete = 4,
  };

  HttpRequest();
  ~HttpRequest();

  /// @brief 设置版本
  /// @param version 版本
  void setVersion(const std::string &version) {
    version_ = version;
  }

  /// @brief 获取版本
  /// @return 版本
  const std::string &version() const {
    return version_;
  }

  /// @brief 设置请求方法
  /// @param start 请求方法字符串起始位置
  /// @param end 请求方法字符串终止位置
  /// @return 是否设置成功
  bool setMethod(const std::string &method);

  /// @brief 获取请求方法
  /// @return 请求方法
  const Method method() const {
    return method_;
  }

  /// @brief 获取请求方法对应字符串
  /// @return 请求方法对应字符串
  const char *methodString() const;

  /// @brief 设置路径
  void setPath(const std::string &path);

  /// @brief 获取路径
  /// @return 路径
  const std::string &path() const {
    return path_;
  }

  void setBody(const std::string &body) {
    body_ = body;
  }

  const std::string &body() const {
    return body_;
  }

  /// @brief 设置请求
  void setQuery(const std::string &query);

  /// @brief 获取请求字符串
  /// @return 请求字符串
  const std::string &query() const {
    return query_;
  }

  /// @brief 设置接收到请求的时间戳
  /// @param ts 接收到请求的时间戳
  void setReceiveTime(const Timestamp &ts) {
    recv_time_ = ts;
  }

  /// @brief 获取接收到请求的时间戳
  /// @return 接收到请求的时间戳
  const Timestamp recvTime() const {
    return recv_time_;
  }

  /// @brief 添加请求头信息
  void AddHeader(const std::string &key, const std::string &value);
  
  /// @brief 获取头信息
  /// @param field 头信息的键
  /// @return 头信息的值
  const std::string getHeader(const std::string &field) const;

  /// @brief 获取所有存储的头信息
  /// @return 所有存储的头信息
  const std::unordered_map<std::string, std::string> &headers() const {
    return headers_;
  }

  void Reset();

  const bool IsKeepAlive() const;

private:
  Method method_;
  std::string version_;
  std::string path_;
  std::string body_;
  std::string query_;
  Timestamp recv_time_;
  std::unordered_map<std::string, std::string> headers_;
};

NAMESPACE_END
