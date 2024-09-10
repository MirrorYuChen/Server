/*
 * @Author: chenjingyu
 * @Date: 2024-09-10 10:11:38
 * @Contact: 2458006466@qq.com
 * @Description: Thread
 */
#pragma once

#include "Api.h"
#include <thread>
#include <functional>
#include <memory>
#include <string>
#include <atomic>

NAMESPACE_BEGIN
class API Thread {
public:
  using ThreadFunc = std::function<void()>;
  explicit Thread(ThreadFunc func, const std::string &name=std::string());
  ~Thread();

  void Start(); // 启动线程
  void Join();  // 等待线程

  bool started() const {
    return started_;
  }

  pid_t tid() const {
    return tid_;
  }

  const std::string &name() const {
    return name_;
  }

  static int numCreated() {
    return num_created_;
  }

private:
  NOT_ALLOWED_COPY(Thread)

private:
  void setDefaultName();                  // 设置线程名

private:
  bool started_;                          // 是否启动线程
  bool joined_;                           // 是否等待线程
  std::shared_ptr<std::thread> thread_;   // 线程
  pid_t tid_;                             // 线程tid
  ThreadFunc func_;                       // 线程函数

  std::string name_;                       // 线程名
  static std::atomic_int32_t num_created_; // 线程索引
};

NAMESPACE_END
