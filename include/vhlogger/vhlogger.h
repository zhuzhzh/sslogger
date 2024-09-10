// vhlogger.h
#ifndef VGP_VHLOGGER_H_
#define VGP_VHLOGGER_H_

#include <iostream>
#include <ostream>
#include <algorithm>
#include <tl/optional.hpp>
#include <spdlog/spdlog.h>
#include <fmt/ranges.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async.h>
#include <spdlog/async_logger.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/fmt/bin_to_hex.h>
#include <memory>
#include <vector>
#include <functional>
#include <string>
#include <unordered_map>

#define VHLOGGER_TRACE    spdlog::level::trace
#define VHLOGGER_DEBUG    spdlog::level::debug
#define VHLOGGER_INFO     spdlog::level::info
#define VHLOGGER_WARN     spdlog::level::warn
#define VHLOGGER_ERROR    spdlog::level::err
#define VHLOGGER_FATAL    spdlog::level::critical
#define VHLOGGER_OFF      spdlog::level::off

namespace vgp {

  class Logger {
  public:
    enum class Verbose { kLite, kLow, kMedium, kHigh, kFull, kUltra};

    using CallbackFunction = std::function<void(const spdlog::details::log_msg&)>;

    struct CallbackCondition {
      spdlog::level::level_enum level = VHLOGGER_OFF;
      tl::optional<std::string> file;  
      tl::optional<int> line;
      tl::optional<std::string> function;
      tl::optional<std::string> message;
    };

    static Logger* GetInstance();

    Logger& SetFile(const std::string& filename, bool truncate = true);
    Logger& SetAsyncMode(bool async_mode);
    Logger& SetLevel(spdlog::level::level_enum level);
    Logger& SetVerbose(Verbose ver);

    Logger& AddLogger(const std::string& name, const std::string& filename, bool daily = false);

    void AddCallback(const CallbackCondition& condition, CallbackFunction callback);

    template<typename... Args>
      void Log(spdlog::level::level_enum level, const char* file, int line, const char* func, const char* fmt, const Args&... args) {
        if (async_mode_ && async_logger_) {
          async_logger_->log(spdlog::source_loc{file, line, func}, level, fmt, args...);
        } else {
          console_logger_->log(spdlog::source_loc{file, line, func}, level, fmt, args...);
        }
#ifdef VHLOGGER_ENABLE_CB
        TriggerCallbacks(level, file, line, func, fmt::format(fmt, args...));
#endif
      }

    template<typename... Args>
      void LogF(spdlog::level::level_enum level, const char* file, int line, const char* func, const char* fmt, const Args&... args) {
        if (async_mode_) {
          if(async_logger_) {
            async_logger_->log(spdlog::source_loc{file, line, func}, level, fmt, args...);
          }
        } else {
          if (sync_logger_) {
            sync_logger_->log(spdlog::source_loc{file, line, func}, level, fmt, args...);
          }
        }
#ifdef VHLOGGER_ENABLE_CB
        TriggerCallbacks(level, file, line, func, fmt::format(fmt, args...));
#endif
      }

    template<typename... Args>
      void LogTo(const std::string& logger_name, spdlog::level::level_enum level, const char* file, int line, const char* func, const char* fmt, const Args&... args) {
        auto it = loggers_.find(logger_name);
        if (it != loggers_.end()) {
          it->second->log(spdlog::source_loc{file, line, func}, level, fmt, args...);
        }
#ifdef VHLOGGER_ENABLE_CB
        TriggerCallbacks(level, file, line, func, fmt::format(fmt, args...));
#endif
      }


    void LogArray(spdlog::level::level_enum level, const uint8_t* ptr, int size, bool to_file = false);

  private:
    Logger();

    void SetupFromEnv();

    spdlog::level::level_enum ParseLogLevel(const std::string& level);

    template<typename LoggerType>
      void UpdateLogger(std::shared_ptr<LoggerType>& logger);

    void UpdateLoggers();

    void TriggerCallbacks(spdlog::level::level_enum level, const char* file, int line,
      const char* func, const std::string& message);

    bool MatchesCondition(const CallbackCondition& condition, const spdlog::details::log_msg& msg);

    std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> console_sink_;
    std::shared_ptr<spdlog::sinks::basic_file_sink_mt> file_sink_;
    std::shared_ptr<spdlog::logger> console_logger_;
    std::shared_ptr<spdlog::logger> sync_logger_;
    std::shared_ptr<spdlog::async_logger> async_logger_;
    std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> loggers_;
    std::string pattern_;
    Verbose verbose_;
    spdlog::level::level_enum level_;
    std::string filename_;
    std::vector<std::pair<CallbackCondition, CallbackFunction>> callbacks_;
    bool async_mode_;
  };

} // namespace vgp

// Macro definitions
#define VGP_LOG_ARRAY(level, ptr, size) vgp::Logger::GetInstance()->LogArray(level, ptr, size, false)
#define VGP_LOG_ARRAY_F(level, ptr, size) vgp::Logger::GetInstance()->LogArray(level, ptr, size, true)

#define VGP_LOG(level, ...) vgp::Logger::GetInstance()->Log(level, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define VGP_TRACE(...) VGP_LOG(VHLOGGER_TRACE, __VA_ARGS__)
#define VGP_DEBUG(...) VGP_LOG(VHLOGGER_DEBUG, __VA_ARGS__)
#define VGP_INFO(...) VGP_LOG(VHLOGGER_INFO, __VA_ARGS__)
#define VGP_WARN(...) VGP_LOG(VHLOGGER_WARN, __VA_ARGS__)
#define VGP_ERROR(...) VGP_LOG(VHLOGGER_ERROR, __VA_ARGS__)
#define VGP_FATAL(...) VGP_LOG(VHLOGGER_FATAL, __VA_ARGS__)

// File logging macros (if needed)
#define VGP_LOG_TO(logger_name, level, ...) vgp::Logger::GetInstance()->LogTo(logger_name, level, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define VGP_TRACE_TO(logger_name, ...) VGP_LOG_TO(logger_name, VHLOGGER_TRACE, __VA_ARGS__)
#define VGP_DEBUG_TO(logger_name, ...) VGP_LOG_TO(logger_name, VHLOGGER_DEBUG, __VA_ARGS__)
#define VGP_INFO_TO(logger_name, ...) VGP_LOG_TO(logger_name, VHLOGGER_INFO, __VA_ARGS__)
#define VGP_WARN_TO(logger_name, ...) VGP_LOG_TO(logger_name, VHLOGGER_WARN, __VA_ARGS__)
#define VGP_ERROR_TO(logger_name, ...) VGP_LOG_TO(logger_name, VHLOGGER_ERROR, __VA_ARGS__)
#define VGP_FATAL_TO(logger_name, ...) VGP_LOG_TO(logger_name, VHLOGGER_FATAL, __VA_ARGS__)

#define VGP_LOG_F(level, ...) vgp::Logger::GetInstance()->LogF(level, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define VGP_TRACEF(...) VGP_LOG_F(VHLOGGER_TRACE, __VA_ARGS__)
#define VGP_DEBUGF(...) VGP_LOG_F(VHLOGGER_DEBUG, __VA_ARGS__)
#define VGP_INFOF(...) VGP_LOG_F(VHLOGGER_INFO, __VA_ARGS__)
#define VGP_WARNF(...) VGP_LOG_F(VHLOGGER_WARN, __VA_ARGS__)
#define VGP_ERRORF(...) VGP_LOG_F(VHLOGGER_ERROR, __VA_ARGS__)
#define VGP_FATALF(...) VGP_LOG_F(VHLOGGER_FATAL, __VA_ARGS__)

#endif  // VGP_VHLOGGER_H_
