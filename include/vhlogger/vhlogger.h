// vhlogger.h
#ifndef VGP_VHLOGGER_H_
#define VGP_VHLOGGER_H_

#include <fmt/format.h>
#include <fmt/chrono.h>
#include <fmt/ranges.h>
#include <fmt/compile.h>
#include <atomic>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <chrono>
#include <functional>
#include <unordered_map>
#include <vector>

// Check for C++17 support
#if __cplusplus >= 201703L
  #define CPP17_OR_GREATER
#endif

#ifdef CPP17_OR_GREATER
#include <optional>
#else
#include "tl/optional.hpp"
#endif

#ifndef VHLOGGER_COMPILE_LEVEL
#define VHLOGGER_COMPILE_LEVEL 3
#endif

namespace vgp {

enum class LOG_LEVEL {
  OFF = 0,
  FATAL=1,
  ERROR,
  WARN,
  INFO,
  DEBUG,
  TRACK
};

// Add a conversion function
inline int to_int(LOG_LEVEL level) {
  return static_cast<int>(level);
}

struct LogContext {
  int level;
  std::string file;
  int line;
  std::string message;
};

class Logger {
 public:
  enum class Format { kLite, kMedium, kFull };

  using CallbackFunction = std::function<void(const LogContext&)>;
  using CallbackId = std::size_t;

  struct CallbackInfo {
    CallbackFunction func;
    int level;
#ifdef CPP17_OR_GREATER
    std::optional<std::string> message;
    std::optional<std::string> file;
    std::optional<int> line;
#else
    tl::optional<std::string> message;
    tl::optional<std::string> file;
    tl::optional<int> line;
#endif
  };

  static Logger& GetInstance();

  void SetFormat(Format format);
  void SetLogFile(const std::string& filename, bool append = false);

  template <typename... Args>
  void LogToConsole(int level, const char* file, int line, const char* format, Args&&... args);

  template <typename... Args>
  void LogToFile(int level, const char* file, int line, const char* format, Args&&... args);

  CallbackId AddCallback(CallbackFunction func, int level, 
#ifdef CPP17_OR_GREATER
                   std::optional<std::string> message = std::nullopt,
                   std::optional<std::string> file = std::nullopt,
                   std::optional<int> line = std::nullopt);
#else
                   tl::optional<std::string> message = tl::nullopt,
                   tl::optional<std::string> file = tl::nullopt,
                   tl::optional<int> line = tl::nullopt);
#endif
  bool RemoveCallback(CallbackId id);

  void ClearCallbacks(int level, 
#ifdef CPP17_OR_GREATER
                      std::optional<std::string> message = std::nullopt,
                      std::optional<std::string> file = std::nullopt,
                      std::optional<int> line = std::nullopt);
#else
                      tl::optional<std::string> message = tl::nullopt,
                      tl::optional<std::string> file = tl::nullopt,
                      tl::optional<int> line = tl::nullopt);
#endif
 private:
    Logger() : verbose_level_(static_cast<int>(LOG_LEVEL::WARN)), format_(Format::kLite) {
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
  void TriggerCallbacks(const LogContext& context);

  std::atomic<int> verbose_level_;
  Format format_;
  std::mutex mutex_;
  std::ofstream log_file_;
  std::unordered_map<CallbackId, CallbackInfo> callbacks_;
  CallbackId next_callback_id_;
};

template <typename... Args>
void Logger::LogToConsole(int level, const char* file, int line, const char* format, Args&&... args) {
  if (level <= verbose_level_) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string message = fmt::format(format, std::forward<Args>(args)...);
    std::string formatted_message = FormatMessage(level, file, line, message);
    std::cout << formatted_message << std::endl;
    LogContext context{level, file, line, message};
    TriggerCallbacks(context);
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
    LogContext context{level, file, line, message};
    TriggerCallbacks(context);
  }
}

#define LOG(level, ...) \
  vgp::Logger::GetInstance().LogToConsole(level, __FILE__, __LINE__, __VA_ARGS__)

#define LOGF(level, ...) \
  vgp::Logger::GetInstance().LogToFile(level, __FILE__, __LINE__, __VA_ARGS__)


// compile-time macro

#ifdef CPP17_OR_GREATER
#define CLOG(level, ...) \
  do { \
    if constexpr (level <= VHLOGGER_COMPILE_LEVEL) { \
      vgp::Logger::GetInstance().LogToConsole(level, __FILE__, __LINE__, __VA_ARGS__); \
    } \
  } while (0)

#define CLOGF(level, ...) \
  do { \
    if constexpr (level <= VHLOGGER_COMPILE_LEVEL) { \
      vgp::Logger::GetInstance().LogToFile(level, __FILE__, __LINE__, __VA_ARGS__); \
    } \
  } while (0)
#else
#define CLOG(level, ...) \
  do { \
    if (level <= VHLOGGER_COMPILE_LEVEL) { \
      vgp::Logger::GetInstance().LogToConsole(level, __FILE__, __LINE__, __VA_ARGS__); \
    } \
  } while (0)

#define CLOGF(level, ...) \
  do { \
    if (level <= VHLOGGER_COMPILE_LEVEL) { \
      vgp::Logger::GetInstance().LogToFile(level, __FILE__, __LINE__, __VA_ARGS__); \
    } \
  } while (0)
#endif

}  // namespace vgp

#endif  // VGP_VHLOGGER_H_
