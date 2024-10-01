// sslogger.h
#ifndef SSLN_SSLOGGER_H_
#define SSLN_SSLOGGER_H_

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

// 在文件开头添加这些定义
#define SSLOGGER_TRACE_INT    0
#define SSLOGGER_DEBUG_INT    1
#define SSLOGGER_INFO_INT     2
#define SSLOGGER_WARN_INT     3
#define SSLOGGER_ERROR_INT    4
#define SSLOGGER_FATAL_INT    5
#define SSLOGGER_OFF_INT      6

#define SSLOGGER_TRACE    spdlog::level::trace
#define SSLOGGER_DEBUG    spdlog::level::debug
#define SSLOGGER_INFO     spdlog::level::info
#define SSLOGGER_WARN     spdlog::level::warn
#define SSLOGGER_ERROR    spdlog::level::err
#define SSLOGGER_FATAL    spdlog::level::critical
#define SSLOGGER_OFF      spdlog::level::off

#ifndef SSLN_ACTIVE_LEVEL
#define SSLN_ACTIVE_LEVEL SSLOGGER_INFO_INT
#endif


namespace ssln {

  class Logger {
  public:
    enum class Verbose { kLite, kLow, kMedium, kHigh, kFull, kUltra};

    using CallbackFunction = std::function<void(const spdlog::details::log_msg&)>;

    struct CallbackCondition {
      spdlog::level::level_enum level = SSLOGGER_OFF;
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
#ifdef SSLOGGER_ENABLE_CB
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
#ifdef SSLOGGER_ENABLE_CB
        TriggerCallbacks(level, file, line, func, fmt::format(fmt, args...));
#endif
      }

    template<typename... Args>
      void LogTo(const std::string& logger_name, spdlog::level::level_enum level, const char* file, int line, const char* func, const char* fmt, const Args&... args) {
        auto it = loggers_.find(logger_name);
        if (it != loggers_.end()) {
          it->second->log(spdlog::source_loc{file, line, func}, level, fmt, args...);
        }
#ifdef SSLOGGER_ENABLE_CB
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
#define SSLN_LOG_ARRAY(level, ptr, size) ssln::Logger::GetInstance()->LogArray(level, ptr, size, false)
#define SSLN_LOGF_ARRAY(level, ptr, size) ssln::Logger::GetInstance()->LogArray(level, ptr, size, true)

#define SSLN_LOG(level, ...) ssln::Logger::GetInstance()->Log(level, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define SSLN_TRACE(...) SSLN_LOG(SSLOGGER_TRACE, __VA_ARGS__)
#define SSLN_DEBUG(...) SSLN_LOG(SSLOGGER_DEBUG, __VA_ARGS__)
#define SSLN_INFO(...) SSLN_LOG(SSLOGGER_INFO, __VA_ARGS__)
#define SSLN_WARN(...) SSLN_LOG(SSLOGGER_WARN, __VA_ARGS__)
#define SSLN_ERROR(...) SSLN_LOG(SSLOGGER_ERROR, __VA_ARGS__)
#define SSLN_FATAL(...) SSLN_LOG(SSLOGGER_FATAL, __VA_ARGS__)

// File logging macros (if needed)
#define SSLN_LOG_TO(logger_name, level, ...) ssln::Logger::GetInstance()->LogTo(logger_name, level, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define SSLN_TRACE_TO(logger_name, ...) SSLN_LOG_TO(logger_name, SSLOGGER_TRACE, __VA_ARGS__)
#define SSLN_DEBUG_TO(logger_name, ...) SSLN_LOG_TO(logger_name, SSLOGGER_DEBUG, __VA_ARGS__)
#define SSLN_INFO_TO(logger_name, ...) SSLN_LOG_TO(logger_name, SSLOGGER_INFO, __VA_ARGS__)
#define SSLN_WARN_TO(logger_name, ...) SSLN_LOG_TO(logger_name, SSLOGGER_WARN, __VA_ARGS__)
#define SSLN_ERROR_TO(logger_name, ...) SSLN_LOG_TO(logger_name, SSLOGGER_ERROR, __VA_ARGS__)
#define SSLN_FATAL_TO(logger_name, ...) SSLN_LOG_TO(logger_name, SSLOGGER_FATAL, __VA_ARGS__)

#define SSLN_LOG_F(level, ...) ssln::Logger::GetInstance()->LogF(level, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define SSLN_TRACEF(...) SSLN_LOG_F(SSLOGGER_TRACE, __VA_ARGS__)
#define SSLN_DEBUGF(...) SSLN_LOG_F(SSLOGGER_DEBUG, __VA_ARGS__)
#define SSLN_INFOF(...) SSLN_LOG_F(SSLOGGER_INFO, __VA_ARGS__)
#define SSLN_WARNF(...) SSLN_LOG_F(SSLOGGER_WARN, __VA_ARGS__)
#define SSLN_ERRORF(...) SSLN_LOG_F(SSLOGGER_ERROR, __VA_ARGS__)
#define SSLN_FATALF(...) SSLN_LOG_F(SSLOGGER_FATAL, __VA_ARGS__)

// compile time logging
#define SSLN_LOGGER_CALL(level, ...) \
    if (level >= SSLN_ACTIVE_LEVEL) SSLN_LOG(level, __VA_ARGS__)

#define SSLN_LOGGER_CALL_F(level, ...) \
    if (level >= SSLN_ACTIVE_LEVEL) SSLN_LOG_F(level, __VA_ARGS__)

#define SSLN_TRACE_ENABLED() (SSLOGGER_TRACE_INT >= SSLN_ACTIVE_LEVEL)
#define SSLN_DEBUG_ENABLED() (SSLOGGER_DEBUG_INT >= SSLN_ACTIVE_LEVEL)
#define SSLN_INFO_ENABLED()  (SSLOGGER_INFO_INT >= SSLN_ACTIVE_LEVEL)
#define SSLN_WARN_ENABLED()  (SSLOGGER_WARN_INT >= SSLN_ACTIVE_LEVEL)
#define SSLN_ERROR_ENABLED() (SSLOGGER_ERROR_INT >= SSLN_ACTIVE_LEVEL)
#define SSLN_FATAL_ENABLED() (SSLOGGER_FATAL_INT >= SSLN_ACTIVE_LEVEL)

#define SSLN_TRACEF_CT(...) SSLN_LOGGER_CALL_F(SSLOGGER_TRACE, __VA_ARGS__)
#define SSLN_DEBUGF_CT(...) SSLN_LOGGER_CALL_F(SSLOGGER_DEBUG, __VA_ARGS__)
#define SSLN_INFOF_CT(...)  SSLN_LOGGER_CALL_F(SSLOGGER_INFO, __VA_ARGS__)
#define SSLN_WARNF_CT(...)  SSLN_LOGGER_CALL_F(SSLOGGER_WARN, __VA_ARGS__)
#define SSLN_ERRORF_CT(...) SSLN_LOGGER_CALL_F(SSLOGGER_ERROR, __VA_ARGS__)
#define SSLN_FATALF_CT(...) SSLN_LOGGER_CALL_F(SSLOGGER_FATAL, __VA_ARGS__)

#if SSLN_ACTIVE_LEVEL <= SSLOGGER_TRACE_INT
#define SSLN_TRACE_CT(...) SSLN_LOGGER_CALL(SSLOGGER_TRACE, __VA_ARGS__)
#else
#define SSLN_TRACE_CT(...) (void)0
#endif

#if SSLN_ACTIVE_LEVEL <= SSLOGGER_DEBUG_INT
#define SSLN_DEBUG_CT(...) SSLN_LOGGER_CALL(SSLOGGER_DEBUG, __VA_ARGS__)
#else
#define SSLN_DEBUG_CT(...) (void)0
#endif

#if SSLN_ACTIVE_LEVEL <= SSLOGGER_INFO_INT
#define SSLN_INFO_CT(...) SSLN_LOGGER_CALL(SSLOGGER_INFO, __VA_ARGS__)
#else
#define SSLN_INFO_CT(...) (void)0
#endif

#if SSLN_ACTIVE_LEVEL <= SSLOGGER_WARN_INT
#define SSLN_WARN_CT(...) SSLN_LOGGER_CALL(SSLOGGER_WARN, __VA_ARGS__)
#else
#define SSLN_WARN_CT(...) (void)0
#endif

#if SSLN_ACTIVE_LEVEL <= SSLOGGER_ERROR_INT
#define SSLN_ERROR_CT(...) SSLN_LOGGER_CALL(SSLOGGER_ERROR, __VA_ARGS__)
#else
#define SSLN_ERROR_CT(...) (void)0
#endif

#if SSLN_ACTIVE_LEVEL <= SSLOGGER_FATAL_INT
#define SSLN_FATAL_CT(...) SSLN_LOGGER_CALL(SSLOGGER_FATAL, __VA_ARGS__)
#else
#define SSLN_FATAL_CT(...) (void)0
#endif

#endif  // SSLN_SSLOGGER_H_
