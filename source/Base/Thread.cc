/*
 * @Author: chenjingyu
 * @Date: 2024-09-10 10:11:44
 * @Contact: 2458006466@qq.com
 * @Description: Thread
 */
#include "Base/Thread.h"
#include "Base/CurrentThread.h"
#include <semaphore.h>
#include <memory>

NAMESPACE_BEGIN
std::atomic_int32_t Thread::num_created_(0);

Thread::Thread(ThreadFunc func, const std::string &name) :
  started_(false), joined_(false), tid_(0), 
  func_(std::move(func)), name_(name) {
  setDefaultName();
}

Thread::~Thread() {
  if (started_ && !joined_) {
    thread_->detach();
  }
}

void Thread::Start() {
  // 标记线程已开始
  started_ = true;
  // 定义信号量，并初始化，初始值未0，表示信号量处于未触发状态，
  // false表示信号量是线程间的，而不是进程间的
  sem_t sem;
  sem_init(&sem, false, 0);

  thread_ = std::make_shared<std::thread>(
    [&]() {
      // 获取线程tid
      tid_ = CurrentThread::tid();
      // 增加信号量的值，从而触发信号量
      // 通知主线程当前线程ID已获取完毕
      sem_post(&sem);
      // 开启一个线程专门处理该线程函数
      func_();
    }
  );
  /**
   * 信号量初始值为0，主线程会在这里阻塞，
   * 直到子线程调用sem_post函数触发信号量，
   * 主线程才会继续执行
   */
  sem_wait(&sem);
}

void Thread::Join() {
  joined_ = true;
  // 等待线程执行完毕
  thread_->join();
}

void Thread::setDefaultName() {
  int num = ++num_created_;
  if (name_.empty()) {
    name_ = "Thread" + std::to_string(num);
  }
}

NAMESPACE_END
