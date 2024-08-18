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

#include "nonstd/string_view.hpp"

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

// 自定义格式化器，用于控制每行32个元素的输出
// 修改后的自定义格式化器
template<>
struct fmt::formatter<std::vector<uint8_t>> {
  constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
    return ctx.end();
  }

  template <typename FormatContext>
    auto format(const std::vector<uint8_t>& vec, FormatContext& ctx) const -> decltype(ctx.out()) {
      fmt::format_to(ctx.out(), "\n");
      for (size_t i = 0; i < vec.size(); ++i) {
        if (i > 0) {
          if (i % 32 == 0) {
            fmt::format_to(ctx.out(), "\n");
          } else {
            fmt::format_to(ctx.out(), " ");
          }
        }
        fmt::format_to(ctx.out(), "{:02X}", vec[i]);
      }
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
    enum class Level :int {OFF=0,  FATAL=1, ERROR, WARN, INFO, DEBUG, TRACE};
    enum class Format :int { kLite, kMedium, kFull };
    const char* ToString(Level level);

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

    Logger& SetFormat(Format format);
    Logger& SetLogFile(const std::string& filename, bool append = false);
    Logger& SetLogLevel(Level verbose);
    Logger& SetLogLevel(int verbose);

    const Format GetFormat() const;
    const std::string GetLogFile() const;
    const Level GetLogLevel() const;

    template<typename... Args>
    void Log(const source_location& loc, Level level, bool to_file, fmt::format_string<Args...> fmt, Args&&... args)
    {
        if (level <= current_level_.load()) {
            auto msg = fmt::format(fmt, std::forward<Args>(args)...);
            LogImpl(loc, level, msg, to_file);
        }
    }

    // Helper struct to capture source location
    struct logger_loc {
        constexpr logger_loc(const source_location& loc) : loc_(loc) {}
        const source_location& loc_;
    };

    void LogArray(const logger_loc& loc, Level level, bool to_file, const uint8_t* ptr, size_t sz);

    // Overloaded logging functions that accept logger_loc
    template<typename... Args>
    void Log(const logger_loc& loc, Level level, bool to_file, fmt::format_string<Args...> fmt, Args&&... args)
    {
        Log(loc.loc_, level, to_file, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void Trace(const logger_loc& loc, bool to_file, fmt::format_string<Args...> fmt, Args&&... args)
    {
        Log(loc.loc_, Level::TRACE, to_file, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void Debug(const logger_loc& loc, bool to_file, fmt::format_string<Args...> fmt, Args&&... args)
    {
        Log(loc.loc_, Level::DEBUG, to_file, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void Info(const logger_loc& loc, bool to_file, fmt::format_string<Args...> fmt, Args&&... args)
    {
        Log(loc.loc_, Level::INFO, to_file, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void Warn(const logger_loc& loc, bool to_file, fmt::format_string<Args...> fmt, Args&&... args)
    {
        Log(loc.loc_, Level::WARN, to_file, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void Error(const logger_loc& loc, bool to_file, fmt::format_string<Args...> fmt, Args&&... args)
    {
        Log(loc.loc_, Level::ERROR, to_file, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void Critical(const logger_loc& loc, bool to_file, fmt::format_string<Args...> fmt, Args&&... args)
    {
        Log(loc.loc_, Level::FATAL, to_file, fmt, std::forward<Args>(args)...);
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
      const char* env_level = std::getenv("SSLN_LOG_LEVEL");
      if (env_level) {
        current_level_ = ParseLogLevel(env_level);
      } else {
        current_level_ = Level::INFO;
      }
      const char* env_logfile = std::getenv("SSLN_LOG_FILE");
      if (env_logfile) {
        SetLogFile(env_logfile);
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
    Level ParseLogLevel(const std::string& level);

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
    std::string file_name_;
    std::unordered_map<CallbackId, CallbackInfo> callbacks_;
    CallbackId next_callback_id_;

    void LogImpl(const source_location& loc, Level level, const std::string& msg, bool to_file=true);
  };

  // New function definitions for logging

// Update the VGP_LOG macro
#define VGP_LOG(level, ...) \
    vgp::logger().Log(VGP_LOG_LOC, level, false, __VA_ARGS__)

// Update the VGP_LOGF macro
#define VGP_LOGF(level, ...) \
    vgp::logger().Log(VGP_LOG_LOC, level, true, __VA_ARGS__)

#define VGP_LOG_ARRAY(level, ptr, sz) \
  vgp::logger().LogArray(VGP_LOG_LOC, level, false, ptr, sz)

#define VGP_LOGF_ARRAY(level, ptr, sz) \
  vgp::logger().LogArray(VGP_LOG_LOC, level, true, ptr, sz)

// Update the compile-time macros

#ifdef CPP17_OR_GREATER
#define VGP_CLOG(level, ...) \
  do { \
    if constexpr (static_cast<int>(level) <= VHLOGGER_COMPILE_LEVEL) { \
      vgp::logger().Log(VGP_LOG_LOC, level, false, __VA_ARGS__); \
    } \
  } while (0)

#define VGP_CLOGF(level, ...) \
  do { \
    if constexpr (static_cast<int>(level) <= VHLOGGER_COMPILE_LEVEL) { \
      vgp::logger().Log(VGP_LOG_LOC, level, true, __VA_ARGS__); \
    } \
  } while (0)

#define VGP_CLOGF_ARRAY(level, ptr, sz) \
  do { \
    if constexpr (static_cast<int>(level) <= VHLOGGER_COMPILE_LEVEL) { \
      vgp::logger().LogArray(VGP_LOG_LOC, level, true, ptr, sz); \
    } \
  } while (0)
#else
#define VGP_CLOG(level, ...) \
  do { \
    if (static_cast<int>(level) <= VHLOGGER_COMPILE_LEVEL) { \
      vgp::logger().Log(VGP_LOG_LOC, level, false, __VA_ARGS__); \
    } \
  } while (0)

#define VGP_CLOGF(level, ...) \
  do { \
    if (static_cast<int>(level) <= VHLOGGER_COMPILE_LEVEL) { \
      vgp::logger().Log(VGP_LOG_LOC, level, true,  __VA_ARGS__); \
    } \
  } while (0)
#define VGP_CLOGF_ARRAY(level, ptr, sz) \
  do { \
    if (static_cast<int>(level) <= VHLOGGER_COMPILE_LEVEL) { \
      vgp::logger().LogArray(VGP_LOG_LOC, level, true, ptr, sz); \
    } \
  } while (0)
#endif

  inline Logger& logger() {
    return Logger::GetInstance();
  }

}  // namespace vgp

// Define a macro to create a logger_loc object with current source location
#define VGP_LOG_LOC vgp::Logger::logger_loc{vgp::source_location{}}

// Update the macros to use VGP_LOG_LOC
#define VGP_TRACE(...) vgp::logger().Trace(VGP_LOG_LOC, false, __VA_ARGS__)
#define VGP_DEBUG(...) vgp::logger().Debug(VGP_LOG_LOC, false, __VA_ARGS__)
#define VGP_INFO(...) vgp::logger().Info(VGP_LOG_LOC, false, __VA_ARGS__)
#define VGP_WARN(...) vgp::logger().Warn(VGP_LOG_LOC, false, __VA_ARGS__)
#define VGP_ERROR(...) vgp::logger().Error(VGP_LOG_LOC, false, __VA_ARGS__)
#define VGP_CRITICAL(...) vgp::logger().Critical(VGP_LOG_LOC, false, __VA_ARGS__)

#define VGP_TRACEF(...) vgp::logger().Trace(VGP_LOG_LOC, true, __VA_ARGS__)
#define VGP_DEBUGF(...) vgp::logger().Debug(VGP_LOG_LOC, true, __VA_ARGS__)
#define VGP_INFOF(...) vgp::logger().Info(VGP_LOG_LOC, true, __VA_ARGS__)
#define VGP_WARNF(...) vgp::logger().Warn(VGP_LOG_LOC, true, __VA_ARGS__)
#define VGP_ERRORF(...) vgp::logger().Error(VGP_LOG_LOC, true, __VA_ARGS__)
#define VGP_CRITICALF(...) vgp::logger().Critical(VGP_LOG_LOC, true, __VA_ARGS__)


#endif  // VGP_VHLOGGER_H_
