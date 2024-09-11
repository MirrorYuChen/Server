/*
 * @Author: chenjingyu
 * @Date: 2024-05-27 14:37:45
 * @Contact: 2458006466@qq.com
 * @Description: Logger
 */
#pragma once

#include "Api.h"
#include <spdlog.h>
#include <sinks/rotating_file_sink.h>
#include <sinks/stdout_color_sinks.h>
#include <chrono>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdarg.h>
#include <string>

#if OS_WINDOWS
#define FILENAME                                                               \
  (strrchr(__FILE__, '\\') ? (strrchr(__FILE__, '\\') + 1) : __FILE__)
#else
#define FILENAME                                                               \
  (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__)
#endif

NAMESPACE_BEGIN
class API Logger {
public:
  Logger();
  ~Logger();

  void setLogPathImpl(const char *log_file_path);

  void setLogTagImpl(const char *tag);

  void setLogLevelImpl(int level);

  spdlog::logger *logger();

  [[gnu::constructor]] static Logger *getInstance();

private:
  NOT_ALLOWED_COPY(Logger)

private:
  std::string log_file_;
  std::string tag_ = "chenjingyu";
  int level_ = 2;
  std::once_flag flag_;
};

class API LogStreamImpl {
public:
  LogStreamImpl(int level, const char *file, int line, bool abort_flag = false);

  ~LogStreamImpl();
  std::ostream &stream();

private:
  NOT_ALLOWED_COPY(LogStreamImpl)

private:
  int level_;
  std::string file_;
  int line_;
  bool abort_flag_;
  std::stringstream log_stream_;
};

inline API void LogPrintfImpl(int level, const char *fmt, ...);

NAMESPACE_END

#define setLogPath(...)                                                        \
  NAMESPACE::Logger::getInstance()->setLogPathImpl(__VA_ARGS__);
#define setLogTag(...)                                                         \
  NAMESPACE::Logger::getInstance()->setLogTagImpl(__VA_ARGS__);
#define setLogLevel(...)                                                       \
  NAMESPACE::Logger::getInstance()->setLogLevelImpl(__VA_ARGS__);

// clang-format on
#define CHECK(x)                                                               \
  if (!(x))                                                                    \
  NAMESPACE::LogStreamImpl(4, FILENAME, __LINE__, true).stream()               \
      << "Check failed: " #x << ": " // NOLINT(*)

#define CHECK_BINARY_IMPL(x, cmp, y) CHECK(x cmp y) << x << #cmp << y << " "
#define CHECK_EQ(x, y) CHECK_BINARY_IMPL(x, ==, y)
#define CHECK_NE(x, y) CHECK_BINARY_IMPL(x, !=, y)
#define CHECK_LT(x, y) CHECK_BINARY_IMPL(x, <, y)
#define CHECK_LE(x, y) CHECK_BINARY_IMPL(x, <=, y)
#define CHECK_GT(x, y) CHECK_BINARY_IMPL(x, >, y)
#define CHECK_GE(x, y) CHECK_BINARY_IMPL(x, >=, y)

#define LTrace 0
#define LDebug 1
#define LInfo 2
#define LWarn 3
#define LError 4
#define LCritical 5

#define LogFormat(LEVEL, fmt, ...)                                             \
  NAMESPACE::Logger::getInstance()->logger()->log(                         \
      spdlog::level::level_enum(LEVEL), "[{}: {}] " fmt, FILENAME, __LINE__,   \
      ##__VA_ARGS__)

#define LogStream(LEVEL)                                                       \
  NAMESPACE::LogStreamImpl(LEVEL, FILENAME, __LINE__).stream()

#define LogPrintf(LEVEL, fmt, ...)                                             \
  NAMESPACE::LogPrintfImpl(LEVEL, "[%s: %d] " fmt, FILENAME, __LINE__,         \
                           ##__VA_ARGS__)

#define LogTrace(fmt, ...) LogFormat(LTrace, fmt, ##__VA_ARGS__)
#define LogDebug(fmt, ...) LogFormat(LDebug, fmt, ##__VA_ARGS__)
#define LogInfo(fmt, ...) LogFormat(LInfo, fmt, ##__VA_ARGS__)
#define LogWarn(fmt, ...) LogFormat(LWarn, fmt, ##__VA_ARGS__)
#define LogError(fmt, ...) LogFormat(LError, fmt, ##__VA_ARGS__)
#define LogCritical(fmt, ...) LogFormat(LCritical, fmt, ##__VA_ARGS__)
