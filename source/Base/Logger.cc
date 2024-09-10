/*
 * @Author: chenjingyu
 * @Date: 2024-09-10 09:57:55
 * @Contact: 2458006466@qq.com
 * @Description: Logger
 */
#include "Base/Logger.h"

NAMESPACE_BEGIN
Logger::Logger() = default;
Logger::~Logger() { spdlog::shutdown(); }

void Logger::setLogPathImpl(const char *log_file_path) {
  log_file_ = std::string(log_file_path);
}

void Logger::setLogTagImpl(const char *tag) { tag_ = std::string(tag); }

void Logger::setLogLevelImpl(int level) { level_ = level; }

spdlog::logger *Logger::logger() {
  static std::shared_ptr<spdlog::logger> instance;
  std::call_once(flag_, [this]() {
    std::vector<spdlog::sink_ptr> sink_vec;
    sink_vec.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    if (!log_file_.empty()) {
      sink_vec.push_back(
        std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
          log_file_, 1024 * 1024 * 5, 5
        )
      );
    }
    auto level = static_cast<spdlog::level::level_enum>(level_);
    for (auto &sink : sink_vec) {
      sink->set_level(level);
    }
    instance = std::make_shared<spdlog::logger>(
        tag_.c_str(), sink_vec.begin(), sink_vec.end());
    instance->set_level(level);
    spdlog::register_logger(instance);
    spdlog::flush_every(std::chrono::seconds(3));
  });
  return instance.get();
}

Logger *Logger::getInstance() {
  static Logger log_wrapper;
  return &log_wrapper;
}


LogStreamImpl::LogStreamImpl(int level, const char *file, int line, bool abort_flag)
  : level_(level), file_(file), line_(line), abort_flag_(abort_flag) {
  log_stream_ << "[" << file_ << ": " << line_ << "] ";
}

LogStreamImpl::~LogStreamImpl() {
  Logger::getInstance()->logger()->log(
    spdlog::level::level_enum(level_),
    log_stream_.str().c_str()
  );
  if (abort_flag_) {
    abort();
  }
}

std::ostream &LogStreamImpl::stream() { return log_stream_; }

static std::string StringPrintf(const char *format, va_list args) {
  std::string output;
  // 1.try a small buffer and hope it fits
  char space[128];
  va_list args_backup;
  va_copy(args_backup, args);
  int bytes = vsnprintf(space, sizeof(space), format, args_backup);
  va_end(args_backup);

  if ((bytes >= 0) && (static_cast<size_t>(bytes) < sizeof(space))) {
    output.append(space, bytes);
    return output;
  }

  // 2.Repeatedly increase buffer size until it fits.
  int length = sizeof(space);
  while (true) {
    if (bytes < 0) {
      length *= 2;
    } else {
      length = bytes + 1;
    }
    char *buf = new char[length];
    // 2.1.Restore the va_list before we use it again
    va_copy(args_backup, args);
    bytes = vsnprintf(buf, length, format, args_backup);
    va_end(args_backup);

    if ((bytes >= 0) && (bytes < length)) {
      output.append(buf, bytes);
      delete[] buf;
      break;
    } else {
      delete[] buf;
    }
  }
  return output;
}

void LogPrintfImpl(int level, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  std::string msg = StringPrintf(fmt, args);
  va_end(args);

  Logger::getInstance()->logger()->log(
    spdlog::level::level_enum(level), msg.c_str()
  );
}

NAMESPACE_END
