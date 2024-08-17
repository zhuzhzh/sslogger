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
#include <string_view>
#else
#include "tl/optional.hpp"
#endif

#ifndef VHLOGGER_COMPILE_LEVEL
#define VHLOGGER_COMPILE_LEVEL 3
#endif

// 自定义格式化器，用于控制每行16个元素的输出
// 修改后的自定义格式化器
template<>
struct fmt::formatter<std::vector<uint8_t>> {
  constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
    return ctx.end();
  }

  template <typename FormatContext>
    auto format(const std::vector<uint8_t>& vec, FormatContext& ctx) const -> decltype(ctx.out()) {
      for (size_t i = 0; i < vec.size(); ++i) {
        if (i > 0) {
          if (i % 16 == 0) {
            fmt::format_to(ctx.out(), "\n");
          } else {
            fmt::format_to(ctx.out(), " ");
          }
        }
        fmt::format_to(ctx.out(), "{:02X}", vec[i]);
      }
      fmt::format_to(ctx.out(), "\n");
      return ctx.out();
    }
};

namespace vgp {

  struct source_location
  {
    constexpr source_location(const char* file = __builtin_FILE(),
      int line = __builtin_LINE(),
      const char* function = __builtin_FUNCTION())
      : file_(file), line_(line), function_(function) {}

    constexpr const char* file_name() const { return file_; }
    constexpr int line() const { return line_; }
    constexpr const char* function_name() const { return function_; }

  private:
    const char* file_;
    int line_;
    const char* function_;
  };

  struct LogContext {
    int level;
    std::string file;
    int line;
    std::string message;
  };


  class Logger {
  public:
    enum class Level {OFF=0,  FATAL, ERROR, WARN, INFO, DEBUG, TRACE};
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
    void SetLogVerbose(int verbose);
    void SetLogLevel(Level verbose);
    void SetLogLevel(int verbose);

    template <typename... Args>
      void LogToConsole(Level level, const char* caller_file, int caller_line, const char* format, Args&&... args);

    template <typename... Args>
      void LogToFile(Level level, const char* caller_file, int caller_line, const char* format, Args&&... args);

    template <typename... Args>
      void LogToConsoleNoNewLine(Level level, const char* caller_file, int caller_line, const char* format, Args&&... args);

    template <typename... Args>
      void LogToFileNoNewLine(Level level, const char* caller_file, int caller_line, fmt::format_string<Args...> format, Args&&... args);

    void LogArrayToFile(Level level, const char* caller_file, int caller_line, const uint8_t* ptr, size_t sz);

    // implement for spdlog-style api
    template<typename... Args>
      void log(const source_location& loc, Level level, fmt::format_string<Args...> fmt, Args&&... args)
      {
        if (level <= current_level_.load()) {
          auto msg = fmt::format(fmt, std::forward<Args>(args)...);
          log_impl(loc, level, msg);
        }
      }

    template<typename... Args>
      void trace(fmt::format_string<Args...> fmt, Args&&... args)
      {
        log(source_location{}, Level::TRACE, fmt, std::forward<Args>(args)...);
      }

    template<typename... Args>
      void debug(fmt::format_string<Args...> fmt, Args&&... args)
      {
        log(source_location{}, Level::DEBUG, fmt, std::forward<Args>(args)...);
      }

    template<typename... Args>
      void info(fmt::format_string<Args...> fmt, Args&&... args)
      {
        log(source_location{}, Level::INFO, fmt, std::forward<Args>(args)...);
      }

    template<typename... Args>
      void warn(fmt::format_string<Args...> fmt, Args&&... args)
      {
        log(source_location{}, Level::WARN, fmt, std::forward<Args>(args)...);
      }

    template<typename... Args>
      void error(fmt::format_string<Args...> fmt, Args&&... args)
      {
        log(source_location{}, Level::ERROR, fmt, std::forward<Args>(args)...);
      }

    template<typename... Args>
      void critical(fmt::format_string<Args...> fmt, Args&&... args)
      {
        log(source_location{}, Level::FATAL, fmt, std::forward<Args>(args)...);
      }



#ifdef CPP17_OR_GREATER
    CallbackId AddCallback(CallbackFunction func, Level level, 
      std::optional<std::string> message = std::nullopt,
      std::optional<std::string> file = std::nullopt,
      std::optional<int> line = std::nullopt);
#else
    CallbackId AddCallback(CallbackFunction func, Level level, 
      tl::optional<std::string> message = tl::nullopt,
      tl::optional<std::string> file = tl::nullopt,
      tl::optional<int> line = tl::nullopt);
#endif
    bool RemoveCallback(CallbackId id);

#ifdef CPP17_OR_GREATER
    void ClearCallbacks(Level level, 
      std::optional<std::string> message = std::nullopt,
      std::optional<std::string> file = std::nullopt,
      std::optional<int> line = std::nullopt);
#else
    void ClearCallbacks(Level level, 
      tl::optional<std::string> message = tl::nullopt,
      tl::optional<std::string> file = tl::nullopt,
      tl::optional<int> line = tl::nullopt);
#endif
  private:
    Logger() : current_level_(Level::INFO), format_(Format::kLite) {
      const char* env_verbose = std::getenv("SSLN_LOG_VERBOSE");
      if (env_verbose) {
        current_level_ = static_cast<Level>(std::atoi(env_verbose));
      } else {
        current_level_ = Level::INFO;
      }
      const char* env_logfile = std::getenv("SSLN_LOG_FILE");
      if (env_logfile) {
        SetLogFile(env_logfile);
      } else {
        SetLogFile("uv_ssln.log");
      }

      const char* env_format = std::getenv("SSLN_LOG_FORMAT");
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
          std::cerr << "Invalid format value in SSLN_LOG_FORMAT: " << format_value
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

    std::string FormatMessage(Level level, const char* file, int line, const std::string& message);
    void TriggerCallbacks(const LogContext& context);

    std::atomic<Level> current_level_;
    Format format_;
    std::mutex mutex_;
    std::ofstream log_file_;
    std::unordered_map<CallbackId, CallbackInfo> callbacks_;
    CallbackId next_callback_id_;

    void log_impl(const source_location& loc, Level level, const std::string& msg);
  };

  template <typename... Args>
    void Logger::LogToConsole(Level level, const char* file, int line, const char* format, Args&&... args) {
      if (level <= current_level_) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string message = fmt::format(format, std::forward<Args>(args)...);
        std::string formatted_message = FormatMessage(level, file, line, message);
        std::cout << formatted_message << std::endl;
        LogContext context{static_cast<int>(level), file, line, message};
        TriggerCallbacks(context);
      }
    }

  template <typename... Args>
    void Logger::LogToFile(Level level, const char* file, int line, const char* format, Args&&... args) {
      if (level <= current_level_) {
        std::lock_guard<std::mutex> lock(mutex_);
        fmt::memory_buffer buf;
        fmt::format_to(std::back_inserter(buf), format, std::forward<Args>(args)...);
        std::string message(buf.data(), buf.size());
        std::string formatted_message = FormatMessage(level, file, line, message);

        if (log_file_.is_open()) {
          log_file_ << formatted_message << std::endl;
          log_file_.flush();
        } else {
          std::cout << formatted_message << std::endl;
          std::cout.flush();
        }
        LogContext context{static_cast<int>(level), file, line, message};
        TriggerCallbacks(context);
      }
    }

  template <typename... Args>
    void Logger::LogToConsoleNoNewLine(Level level, const char* file, int line, const char* format, Args&&... args) {
      if (level <= current_level_) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string message = fmt::format(format, std::forward<Args>(args)...);
        std::string formatted_message = FormatMessage(level, file, line, message);
        std::cout << formatted_message;
        LogContext context{static_cast<int>(level), file, line, message};
        TriggerCallbacks(context);
      }
    }

  template <typename... Args>
    void Logger::LogToFileNoNewLine(Level level, const char* file, int line, fmt::format_string<Args...> format, Args&&... args) {
      if (level <= current_level_) {
        std::lock_guard<std::mutex> lock(mutex_);
        fmt::memory_buffer buf;
        fmt::format_to(std::back_inserter(buf), format, std::forward<Args>(args)...);
#ifdef CPP17_OR_GREATER
        std::string_view message(buf.data(), buf.size());
#else
        std::string message(buf.data(), buf.size());
#endif
        std::string formatted_message = FormatMessage(level, file, line, std::string(message));

        if (log_file_.is_open()) {
          log_file_ << formatted_message;
          log_file_.flush();
        } else {
          std::cout << formatted_message;
          std::cout.flush();
        }
        LogContext context{static_cast<int>(level), file, line, std::string(message)};
        TriggerCallbacks(context);
      }
    }

  // New function definitions for logging

  template <typename... Args>
    void log(Logger::Level level, const char* caller_file, int caller_line, const char* format, Args&&... args) {
      vgp::Logger::GetInstance().LogToConsoleNoNewLine(level, caller_file, caller_line, format, std::forward<Args>(args)...);
    }

  template <typename... Args>
    void logf(Logger::Level level, const char* caller_file, int caller_line, fmt::format_string<Args...> format, Args&&... args) {
      vgp::Logger::GetInstance().LogToFileNoNewLine(level, caller_file, caller_line, format, std::forward<Args>(args)...);
    }

  template <typename... Args>
    void logn(Logger::Level level, const char* caller_file, int caller_line, const char* format, Args&&... args) {
      vgp::Logger::GetInstance().LogToConsole(level, caller_file, caller_line, format, std::forward<Args>(args)...);
    }

  template <typename... Args>
    void logfn(Logger::Level level, const char* caller_file, int caller_line, const char* format, Args&&... args) {
      vgp::Logger::GetInstance().LogToFile(level, caller_file, caller_line, format, std::forward<Args>(args)...);
    }

  void logf_array(Logger::Level level, const uint8_t* ptr, size_t sz);

#define VGP_LOG(level, ...) \
  vgp::log(level, __FILE__, __LINE__, __VA_ARGS__)

#define VGP_LOGF(level, ...) \
  vgp::logf(level, __FILE__, __LINE__, __VA_ARGS__)

#define VGP_LOGN(level, ...) \
  vgp::logn(level, __FILE__, __LINE__, __VA_ARGS__)

#define VGP_LOGFN(level, ...) \
  vgp::logfn(level, __FILE__, __LINE__, __VA_ARGS__)

#define VGP_LOGF_ARRAY(level, ptr, sz) \
  vgp::logf_array(level, __FILE__, __LINE__, ptr, sz)

  // Update the compile-time macros

#ifdef CPP17_OR_GREATER
#define VGP_CLOG(level, ...) \
  do { \
    if constexpr (static_cast<int>(level) <= VHLOGGER_COMPILE_LEVEL) { \
      vgp::Logger::GetInstance().LogToConsoleNoNewLine(level, __FILE__, __LINE__, __VA_ARGS__); \
    } \
  } while (0)

#define VGP_CLOGF(level, ...) \
  do { \
    if constexpr (static_cast<int>(level) <= VHLOGGER_COMPILE_LEVEL) { \
      vgp::Logger::GetInstance().LogToFileNoNewLine(level, __FILE__, __LINE__, __VA_ARGS__); \
    } \
  } while (0)

#define VGP_CLOGF_ARRAY(level, ptr, sz) \
  do { \
    if constexpr (static_cast<int>(level) <= VHLOGGER_COMPILE_LEVEL) { \
      vgp::Logger::GetInstance().LogArrayToFile(level, __FILE__, __LINE__, ptr, sz); \
    } \
  } while (0)

#define VGP_CLOGN(level, ...) \
  do { \
    if constexpr (static_cast<int>(level) <= VHLOGGER_COMPILE_LEVEL) { \
      vgp::Logger::GetInstance().LogToConsole(level, __FILE__, __LINE__, __VA_ARGS__); \
    } \
  } while (0)

#define VGP_CLOGFN(level, ...) \
  do { \
    if constexpr (static_cast<int>(level) <= VHLOGGER_COMPILE_LEVEL) { \
      vgp::Logger::GetInstance().LogToFile(level, __FILE__, __LINE__, __VA_ARGS__); \
    } \
  } while (0)
#else
#define VGP_CLOG(level, ...) \
  do { \
    if (static_cast<int>(level) <= VHLOGGER_COMPILE_LEVEL) { \
      vgp::Logger::GetInstance().LogToConsoleNoNewLine(level, __FILE__, __LINE__, __VA_ARGS__); \
    } \
  } while (0)

#define VGP_CLOGF(level, ...) \
  do { \
    if (static_cast<int>(level) <= VHLOGGER_COMPILE_LEVEL) { \
      vgp::Logger::GetInstance().LogToFileNoNewLine(level, __FILE__, __LINE__, __VA_ARGS__); \
    } \
  } while (0)
#define VGP_CLOGN(level, ...) \
  do { \
    if (static_cast<int>(level) <= VHLOGGER_COMPILE_LEVEL) { \
      vgp::Logger::GetInstance().LogToConsole(level, __FILE__, __LINE__, __VA_ARGS__); \
    } \
  } while (0)

#define VGP_CLOGFN(level, ...) \
  do { \
    if (static_cast<int>(level) <= VHLOGGER_COMPILE_LEVEL) { \
      vgp::Logger::GetInstance().LogToFile(level, __FILE__, __LINE__, __VA_ARGS__); \
    } \
  } while (0)
#endif


  inline Logger& logger() {
    return Logger::GetInstance();
  }

}  // namespace vgp

#define VGP_TRACE(...) vgp::logger().trace(__VA_ARGS__)
#define VGP_DEBUG(...) vgp::logger().debug(__VA_ARGS__)
#define VGP_INFO(...) vgp::logger().info(__VA_ARGS__)
#define VGP_WARN(...) vgp::logger().warn(__VA_ARGS__)
#define VGP_ERROR(...) vgp::logger().error(__VA_ARGS__)
#define VGP_CRITICAL(...) vgp::logger().critical(__VA_ARGS__)

#endif  // VGP_VHLOGGER_H_
