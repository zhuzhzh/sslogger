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

  const int  OFF = 0;
  const int  FATAL=1;
  const int  ERROR=2;
  const int  WARN=3;
  const int  INFO=4;
  const int  DEBUG=5;
  const int  TRACK=6;


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

    // New static method to set verbosity level
    static void SetLogVerbose(int verbose);

    template <typename... Args>
      void LogToConsole(int level, const char* file, int line, const char* format, Args&&... args);

    template <typename... Args>
      void LogToFile(int level, const char* file, int line, const char* format, Args&&... args);

#ifdef CPP17_OR_GREATER
    CallbackId AddCallback(CallbackFunction func, int level, 
      std::optional<std::string> message = std::nullopt,
      std::optional<std::string> file = std::nullopt,
      std::optional<int> line = std::nullopt);
#else
    CallbackId AddCallback(CallbackFunction func, int level, 
      tl::optional<std::string> message = tl::nullopt,
      tl::optional<std::string> file = tl::nullopt,
      tl::optional<int> line = tl::nullopt);
#endif
    bool RemoveCallback(CallbackId id);

#ifdef CPP17_OR_GREATER
    void ClearCallbacks(int level, 
      std::optional<std::string> message = std::nullopt,
      std::optional<std::string> file = std::nullopt,
      std::optional<int> line = std::nullopt);
#else
    void ClearCallbacks(int level, 
      tl::optional<std::string> message = tl::nullopt,
      tl::optional<std::string> file = tl::nullopt,
      tl::optional<int> line = tl::nullopt);
#endif
  private:
    Logger() : verbose_level_(vgp::WARN), format_(Format::kLite) {
      const char* env_verbose = std::getenv("SSLN_LOG_VERBOSE");
      if (env_verbose) {
        verbose_level_ = std::atoi(env_verbose);
      } else {
        verbose_level_ = vgp::INFO;
      }
      const char* env_logfile = std::getenv("SSLN_LOGFILE");
      if (env_logfile) {
        SetLogFile(env_logfile);
      } else {
        SetLogFile("uv_ssln.log");
      }

      const char* env_format = std::getenv("SSLN_LOGGER_FORMAT");
      if (env_format) {
        int format_value = std::atoi(env_format);
        switch (format_value) {
        case 0:
          SetFormat(Format::kLite);
          break;
        case 1:
          SetFormat(Format::kMedium);
          break;
        case 2:
          SetFormat(Format::kFull);
          break;
        default:
          std::cerr << "Invalid format value in UV_SSLN_FORMAT: " << format_value
            << ". Using default format (kLite)." << std::endl;
          SetFormat(Format::kLite);
          break;
        }
      } else {
        SetFormat(Format::kLite);
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


// #define LOG(level, ...) \
//   vgp::Logger::GetInstance().LogToConsole(level, __FILE__, __LINE__, __VA_ARGS__)
// 
// #define LOGF(level, ...) \
//   vgp::Logger::GetInstance().LogToFile(level, __FILE__, __LINE__, __VA_ARGS__)

  // New function definitions for logging

  template <typename... Args>
  void log(int level, const char* format, Args&&... args) {
    vgp::Logger::GetInstance().LogToConsole(level, __FILE__, __LINE__, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void logf(int level, const char* format, Args&&... args) {
    vgp::Logger::GetInstance().LogToFile(level, __FILE__, __LINE__, format, std::forward<Args>(args)...);
  }


  // compile-time macro

#ifdef CPP17_OR_GREATER
#define VGP_CLOG(level, ...) \
  do { \
    if constexpr (level <= VHLOGGER_COMPILE_LEVEL) { \
      vgp::Logger::GetInstance().LogToConsole(level, __FILE__, __LINE__, __VA_ARGS__); \
    } \
  } while (0)

#define VGP_CLOGF(level, ...) \
  do { \
    if constexpr (level <= VHLOGGER_COMPILE_LEVEL) { \
      vgp::Logger::GetInstance().LogToFile(level, __FILE__, __LINE__, __VA_ARGS__); \
    } \
  } while (0)
#else
#define VGP_CLOG(level, ...) \
  do { \
    if (level <= VHLOGGER_COMPILE_LEVEL) { \
      vgp::Logger::GetInstance().LogToConsole(level, __FILE__, __LINE__, __VA_ARGS__); \
    } \
  } while (0)

#define VGP_CLOGF(level, ...) \
  do { \
    if (level <= VHLOGGER_COMPILE_LEVEL) { \
      vgp::Logger::GetInstance().LogToFile(level, __FILE__, __LINE__, __VA_ARGS__); \
    } \
  } while (0)
#endif

}  // namespace vgp

#endif  // VGP_VHLOGGER_H_
