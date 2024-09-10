/*
 * @Author: chenjingyu
 * @Date: 2024-09-10 16:46:25
 * @Contact: 2458006466@qq.com
 * @Description: Timestamp
 */
#pragma once

#include "Api.h"
#include <string>

NAMESPACE_BEGIN
class API Timestamp {
public:
  static constexpr const int kMicroSecondsPerSecond = 1e6;
  Timestamp() : micro_seconds_since_epoch_(0) {}

  explicit Timestamp(int64_t micro_seconds_since_epoch)
      : micro_seconds_since_epoch_(micro_seconds_since_epoch) {}

  // 获取当前时间戳
  static Timestamp Now();

  //用std::string形式返回,格式[millisec].[microsec]
  std::string toString() const;
  //格式, "%4d年%02d月%02d日 星期%d %02d:%02d:%02d.%06d",时分秒.微秒
  std::string toFormattedString(bool showMicroseconds = false) const;

  //返回当前时间戳的微妙
  int64_t microSecondsSinceEpoch() const { return micro_seconds_since_epoch_; }
  //返回当前时间戳的秒数
  time_t secondsSinceEpoch() const {
    return static_cast<time_t>(micro_seconds_since_epoch_ /
                               kMicroSecondsPerSecond);
  }

  static Timestamp invalid() { return Timestamp(); }

private:
  int64_t micro_seconds_since_epoch_;
};

inline bool operator<(Timestamp lhs, Timestamp rhs) {
  return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs) {
  return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

inline Timestamp addTime(Timestamp timestamp, double seconds) {
  int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
  return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}

NAMESPACE_END
