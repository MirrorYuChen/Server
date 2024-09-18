/*
 * @Author: chenjingyu
 * @Date: 2024-09-18 17:29:07
 * @Contact: 2458006466@qq.com
 * @Description: Locker
 */
#include "Base/Locker.h"

NAMESPACE_BEGIN
Sem::Sem() {
  if (sem_init(&sem_, 0, 0) != 0) {
    throw std::exception();
  }
}

Sem::Sem(int num) {
  if (sem_init(&sem_, 0, num) != 0) {
    throw std::exception();
  }
}

Sem::~Sem() {
  sem_destroy(&sem_);
}


bool Sem::Wait() {
  return sem_wait(&sem_) == 0;
}

bool Sem::Post() {
  return sem_post(&sem_) == 0;
}

Locker::Locker() {
  if (pthread_mutex_init(&mtx_, NULL) != 0) {
    throw std::exception();
  }
}

Locker::~Locker() {
  pthread_mutex_destroy(&mtx_);
}


bool Locker::Lock() {
  return pthread_mutex_lock(&mtx_) == 0;
}

bool Locker::Unlock() {
  return pthread_mutex_unlock(&mtx_) == 0;
}

pthread_mutex_t *Locker::get() {
  return &mtx_;
}

Cond::Cond() {
  if (pthread_cond_init(&cond_, NULL) != 0) {
    throw std::exception();
  }
}

Cond::~Cond() {
  pthread_cond_destroy(&cond_);
}


bool Cond::Wait(pthread_mutex_t *mtx) {
  return pthread_cond_wait(&cond_, mtx) == 0;
}

bool Cond::TimeWWait(pthread_mutex_t *mtx, struct timespec t) {
  return pthread_cond_timedwait(&cond_, mtx, &t) == 0;
}

bool Cond::Signal() {
  return pthread_cond_signal(&cond_) == 0;
}

bool Cond::Broadcast() {
  return pthread_cond_broadcast(&cond_) == 0;
}

NAMESPACE_END
