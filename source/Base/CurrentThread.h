/*
 * @Author: chenjingyu
 * @Date: 2024-09-10 10:26:09
 * @Contact: 2458006466@qq.com
 * @Description: CurrentThread
 */
#pragma once

#include "Api.h"
#include <unistd.h>
#include <sys/syscall.h>

NAMESPACE_BEGIN
namespace CurrentThread {
// 保存tid缓冲，避免多次系统调用
// __thread关键字是GCC和Clang编译器提供的一个扩展
// 用于声明线程局部存储(Thread-Local Storage, TLS)
// 这意味着使用__thread修饰的变量在每个线程中都有独立的实例
extern __thread int t_cached_tid;

void API cachedTid();

inline int API tid() {
  if (__builtin_expect(t_cached_tid == 0, 0)) {
    cachedTid();
  }
  return t_cached_tid;
}

} // namespace CurrentThread
NAMESPACE_END
