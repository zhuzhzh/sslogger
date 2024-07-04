// vhlogger.h
#ifndef VGP_VHLOGGER_H_
#define VGP_VHLOGGER_H_

#include <fmt/format.h>
#include <fmt/chrono.h>
#include <atomic>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <chrono>

namespace vgp {

class Logger {
 public:
  enum class Format { kLite, kMedium, kFull };

  static Logger& GetInstance();

  void SetFormat(Format format);
  void SetLogFile(const std::string& filename, bool append = false);

  template <typename... Args>
  void LogToConsole(int level, const char* file, int line, const char* format, Args&&... args);

  template <typename... Args>
  void LogToFile(int level, const char* file, int line, const char* format, Args&&... args);

 private:
    Logger() : verbose_level_(0), format_(Format::kLite) {
    const char* env_verbose = std::getenv("UV_HYBRID_VERBOSE");
    if (env_verbose) {
      verbose_level_ = std::atoi(env_verbose);
    }
  }

  ~Logger() {
    if (log_file_.is_open()) {
      log_file_.close();
    }
  }
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

  std::string FormatMessage(int level, const char* file, int line, const std::string& message);

  std::atomic<int> verbose_level_;
  Format format_;
  std::mutex mutex_;
  std::ofstream log_file_;
};

template <typename... Args>
void Logger::LogToConsole(int level, const char* file, int line, const char* format, Args&&... args) {
  if (level <= verbose_level_) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string message = fmt::format(format, std::forward<Args>(args)...);
    std::string formatted_message = FormatMessage(level, file, line, message);
    std::cout << formatted_message << std::endl;
  }
}

template <typename... Args>
void Logger::LogToFile(int level, const char* file, int line, const char* format, Args&&... args) {
  if (level <= verbose_level_) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string message = fmt::format(format, std::forward<Args>(args)...);
    std::string formatted_message = FormatMessage(level, file, line, message);
    
    if (log_file_.is_open()) {
      log_file_ << formatted_message << std::endl;
    } else {
      std::cout << formatted_message << std::endl;
    }
  }
}

}  // namespace vgp

#define LOG(level, ...) \
  vgp::Logger::GetInstance().LogToConsole(level, __FILE__, __LINE__, __VA_ARGS__)

#define LOGF(level, ...) \
  vgp::Logger::GetInstance().LogToFile(level, __FILE__, __LINE__, __VA_ARGS__)


#endif  // VGP_VHLOGGER_H_
