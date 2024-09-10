/*
 * @Author: chenjingyu
 * @Date: 2024-09-10 16:46:31
 * @Contact: 2458006466@qq.com
 * @Description: Timestamp
 */
#include "Base/Timestamp.h"
#include <sys/time.h>

NAMESPACE_BEGIN
Timestamp Timestamp::Now() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  int64_t seconds = tv.tv_sec;
  return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}

std::string Timestamp::toFormattedString(bool showMicroseconds) const {
  char buf[64] = {0};
  time_t seconds = static_cast<time_t>(micro_seconds_since_epoch_ / kMicroSecondsPerSecond);
  tm *tm_time = localtime(&seconds);
  if (showMicroseconds) {
    int microseconds = static_cast<int>(micro_seconds_since_epoch_ % kMicroSecondsPerSecond);
    snprintf(buf, sizeof(buf), "%4d/%02d/%02d %02d:%02d:%02d.%06d",
             tm_time->tm_year + 1900, tm_time->tm_mon + 1, tm_time->tm_mday,
             tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec, microseconds);
  } else {
    snprintf(buf, sizeof(buf), "%4d/%02d/%02d %02d:%02d:%02d",
             tm_time->tm_year + 1900, tm_time->tm_mon + 1, tm_time->tm_mday,
             tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec);
  }
  return buf;
}

NAMESPACE_END
