/*
 * @Author: chenjingyu
 * @Date: 2024-09-18 17:27:09
 * @Contact: 2458006466@qq.com
 * @Description: Locker
 */
#pragma once

#include "Api.h"
#include <exception>
#include <pthread.h>
#include <semaphore.h>

NAMESPACE_BEGIN
/// @brief 信号量RAII封装
class Sem {
public:
  Sem();
  explicit Sem(int num);
  ~Sem();

  bool Wait();
  bool Post();

private:
  sem_t sem_;
};

/// @brief 互斥锁RAII封装
class Locker {
public:
  Locker();
  ~Locker();

  bool Lock();
  bool Unlock();
  pthread_mutex_t *get();

private:
  pthread_mutex_t mtx_;
};

/// @brief 条件变量RAII封装
class Cond {
public:
  Cond();
  ~Cond();

  bool Wait(pthread_mutex_t *mtx);
  bool TimeWWait(pthread_mutex_t *mtx, struct timespec t);
  bool Signal();
  bool Broadcast();

private:
  pthread_cond_t cond_;
};

NAMESPACE_END
