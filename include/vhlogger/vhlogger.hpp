#ifndef VGP_VHLOGGER_HPP_
#define VGP_VHLOGGER_HPP_

#include <fmt/format.h>
#include <fmt/chrono.h>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>

namespace vgp {

enum class LogLevel {
  DEBUG,
  INFO,
  WARN,
  ERROR,
  FATAL
};

class Logger {
 public:
  enum class Format {
    LITE,   // 只打印消息
    MEDIUM, // 带时间的格式
    FULL    // 默认格式（包含所有信息）
  };

  static Logger& GetInstance();

  void SetFormat(Format format);
  void SetLogFile(const std::string& filename, bool append = false);
  void SetLogLevel(LogLevel level);

  template <typename... Args>
  void Log(LogLevel level, const char* file, int line, const char* format, Args&&... args);

  template <typename... Args>
  void LogF(LogLevel level, const char* file, int line, const char* format, Args&&... args);

 private:
  Logger();
  ~Logger();

  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

  std::string FormatHeader(LogLevel level, const char* file, int line);
  void WriteLog(const std::string& message);

  Format format_ = Format::FULL;
  LogLevel log_level_ = LogLevel::INFO;
  std::mutex mutex_;
  std::ofstream log_file_;
};

#define LOG(level, ...) \
  vgp::Logger::GetInstance().Log(vgp::LogLevel::level, __FILE__, __LINE__, __VA_ARGS__)

#define LOGF(level, ...) \
  vgp::Logger::GetInstance().LogF(vgp::LogLevel::level, __FILE__, __LINE__, __VA_ARGS__)

#define LOG_DEBUG(...) LOG(DEBUG, __VA_ARGS__)
#define LOG_INFO(...)  LOG(INFO, __VA_ARGS__)
#define LOG_WARN(...)  LOG(WARN, __VA_ARGS__)
#define LOG_ERROR(...) LOG(ERROR, __VA_ARGS__)
#define LOG_FATAL(...) LOG(FATAL, __VA_ARGS__)

#define LOGF_DEBUG(...) LOGF(DEBUG, __VA_ARGS__)
#define LOGF_INFO(...)  LOGF(INFO, __VA_ARGS__)
#define LOGF_WARN(...)  LOGF(WARN, __VA_ARGS__)
#define LOGF_ERROR(...) LOGF(ERROR, __VA_ARGS__)
#define LOGF_FATAL(...) LOGF(FATAL, __VA_ARGS__)


template <typename... Args>
void Logger::Log(LogLevel level, const char* file, int line, const char* format, Args&&... args) {
  if (level >= log_level_) {
    std::string header = FormatHeader(level, file, line);
    std::string message = fmt::format(format, std::forward<Args>(args)...);
    WriteLog(header + message);
  }
}

template <typename... Args>
void Logger::LogF(LogLevel level, const char* file, int line, const char* format, Args&&... args) {
  if (level >= log_level_) {
    std::string header = FormatHeader(level, file, line);
    std::string message = fmt::format(format, std::forward<Args>(args)...);
    WriteLog(header + message);
  }
}

}  // namespace vgp


#endif  // VGP_VHLOGGER_HPP_
