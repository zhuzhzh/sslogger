// sslogger.h
#ifndef SSLN_SSLOGGER_H_
#define SSLN_SSLOGGER_H_

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



#define SSLOGGER_TRACE    spdlog::level::trace
#define SSLOGGER_DEBUG    spdlog::level::debug
#define SSLOGGER_INFO     spdlog::level::info
#define SSLOGGER_WARN     spdlog::level::warn
#define SSLOGGER_ERROR    spdlog::level::err
#define SSLOGGER_FATAL    spdlog::level::critical
#define SSLOGGER_OFF      spdlog::level::off

namespace ssln {

// 删除原有的 INT 宏定义，改用枚举类
enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Fatal,
    Off
};

#ifndef SSLN_ACTIVE_LEVEL
#define SSLN_ACTIVE_LEVEL ::ssln::LogLevel::Info
#endif

// 在编译时比较日志级别的模板
template<LogLevel A, LogLevel B>
constexpr bool LogLevelLE() {
    return static_cast<int>(A) <= static_cast<int>(B);
}

// 编译时日志级别检查宏
#define SSLN_LEVEL_ENABLED(level) \
    ::ssln::LogLevelLE<level, SSLN_ACTIVE_LEVEL>()

constexpr spdlog::level::level_enum ToSpdLogLevel(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return SSLOGGER_TRACE;
        case LogLevel::Debug: return SSLOGGER_DEBUG;
        case LogLevel::Info:  return SSLOGGER_INFO;
        case LogLevel::Warn:  return SSLOGGER_WARN;
        case LogLevel::Error: return SSLOGGER_ERROR;
        case LogLevel::Fatal: return SSLOGGER_FATAL;
        case LogLevel::Off:   return SSLOGGER_OFF;
        default:             return SSLOGGER_INFO;
    }
}

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

    Logger();
    ~Logger();

    // 初始化函数
    static void Init(const std::string& log_dir = ".", 
                    const std::string& log_file = "",
                    bool async_mode = false,
                    spdlog::level::level_enum level = SSLOGGER_INFO,
                    Verbose verbose = Verbose::kMedium,
                    bool allow_env_override = true);

    // 关闭函数
    static void Shutdown();    

    Logger& SetFile(const std::string& filename, bool truncate = true);
    Logger& SetAsyncMode(bool async_mode);
    Logger& SetLevel(spdlog::level::level_enum level);
    Logger& SetVerbose(Verbose ver);

    Logger& AddLogger(const std::string& name, const std::string& filename, bool daily = false);

    void AddCallback(const CallbackCondition& condition, CallbackFunction callback);

    template<typename... Args>
      void Log(spdlog::level::level_enum level, const char* file, int line, const char* func, const char* fmt, const Args&... args) {
        if (config_.async_mode && async_logger_) {
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
        if (config_.async_mode) {
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
    void LoadFromEnv();
    
    // 存储初始化配置
    struct Config {
        std::string log_dir = ".";
        std::string log_file;
        bool async_mode = false;
        spdlog::level::level_enum level = SSLOGGER_INFO;
        Verbose verbose = Verbose::kMedium;
        std::string pattern_ = "[%Y-%m-%d %H:%M:%S.%f][%^%L%$][%t][%@] %v";
        bool allow_env_override = true;
    };
    Config config_;

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

    std::vector<std::pair<CallbackCondition, CallbackFunction>> callbacks_;
  };

extern ssln::Logger* g_logger;

} // namespace ssln


// Macro definitions
#define SSLN_LOG_ARRAY(level, ptr, size) ssln::g_logger->LogArray(level, ptr, size, false)
#define SSLN_LOGF_ARRAY(level, ptr, size) ssln::g_logger->LogArray(level, ptr, size, true)

#define SSLN_LOG(level, ...) \
  if(ssln::g_logger) ssln::g_logger->Log(level, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define SSLN_TRACE(...) SSLN_LOG(SSLOGGER_TRACE, __VA_ARGS__)
#define SSLN_DEBUG(...) SSLN_LOG(SSLOGGER_DEBUG, __VA_ARGS__)
#define SSLN_INFO(...) SSLN_LOG(SSLOGGER_INFO, __VA_ARGS__)
#define SSLN_WARN(...) SSLN_LOG(SSLOGGER_WARN, __VA_ARGS__)
#define SSLN_ERROR(...) SSLN_LOG(SSLOGGER_ERROR, __VA_ARGS__)
#define SSLN_FATAL(...) SSLN_LOG(SSLOGGER_FATAL, __VA_ARGS__)

// File logging macros (if needed)
#define SSLN_LOG_TO(logger_name, level, ...) \
  if (ssln::g_logger) ssln::g_logger->LogTo(logger_name, level, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define SSLN_TRACE_TO(logger_name, ...) SSLN_LOG_TO(logger_name, SSLOGGER_TRACE, __VA_ARGS__)
#define SSLN_DEBUG_TO(logger_name, ...) SSLN_LOG_TO(logger_name, SSLOGGER_DEBUG, __VA_ARGS__)
#define SSLN_INFO_TO(logger_name, ...) SSLN_LOG_TO(logger_name, SSLOGGER_INFO, __VA_ARGS__)
#define SSLN_WARN_TO(logger_name, ...) SSLN_LOG_TO(logger_name, SSLOGGER_WARN, __VA_ARGS__)
#define SSLN_ERROR_TO(logger_name, ...) SSLN_LOG_TO(logger_name, SSLOGGER_ERROR, __VA_ARGS__)
#define SSLN_FATAL_TO(logger_name, ...) SSLN_LOG_TO(logger_name, SSLOGGER_FATAL, __VA_ARGS__)

#define SSLN_LOG_F(level, ...) \
  if (ssln::g_logger) ssln::g_logger->LogF(level, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define SSLN_TRACEF(...) SSLN_LOG_F(SSLOGGER_TRACE, __VA_ARGS__)
#define SSLN_DEBUGF(...) SSLN_LOG_F(SSLOGGER_DEBUG, __VA_ARGS__)
#define SSLN_INFOF(...) SSLN_LOG_F(SSLOGGER_INFO, __VA_ARGS__)
#define SSLN_WARNF(...) SSLN_LOG_F(SSLOGGER_WARN, __VA_ARGS__)
#define SSLN_ERRORF(...) SSLN_LOG_F(SSLOGGER_ERROR, __VA_ARGS__)
#define SSLN_FATALF(...) SSLN_LOG_F(SSLOGGER_FATAL, __VA_ARGS__)

// compile time logging
#define SSLN_LOGGER_CALL(level, ...) \
    if constexpr (SSLN_LEVEL_ENABLED(level)) SSLN_LOG(ToSpdLogLevel(level), __VA_ARGS__)

#define SSLN_LOGGER_CALL_F(level, ...) \
    if constexpr (SSLN_LEVEL_ENABLED(level)) SSLN_LOG_F(ToSpdLogLevel(level), __VA_ARGS__)

#define SSLN_TRACE_ENABLED() SSLN_LEVEL_ENABLED(::ssln::LogLevel::Trace)
#define SSLN_DEBUG_ENABLED() SSLN_LEVEL_ENABLED(::ssln::LogLevel::Debug)
#define SSLN_INFO_ENABLED()  SSLN_LEVEL_ENABLED(::ssln::LogLevel::Info)
#define SSLN_WARN_ENABLED()  SSLN_LEVEL_ENABLED(::ssln::LogLevel::Warn)
#define SSLN_ERROR_ENABLED() SSLN_LEVEL_ENABLED(::ssln::LogLevel::Error)
#define SSLN_FATAL_ENABLED() SSLN_LEVEL_ENABLED(::ssln::LogLevel::Fatal)

#define SSLN_TRACEF_CT(...) SSLN_LOGGER_CALL_F(SSLOGGER_TRACE, __VA_ARGS__)
#define SSLN_DEBUGF_CT(...) SSLN_LOGGER_CALL_F(SSLOGGER_DEBUG, __VA_ARGS__)
#define SSLN_INFOF_CT(...)  SSLN_LOGGER_CALL_F(SSLOGGER_INFO, __VA_ARGS__)
#define SSLN_WARNF_CT(...)  SSLN_LOGGER_CALL_F(SSLOGGER_WARN, __VA_ARGS__)
#define SSLN_ERRORF_CT(...) SSLN_LOGGER_CALL_F(SSLOGGER_ERROR, __VA_ARGS__)
#define SSLN_FATALF_CT(...) SSLN_LOGGER_CALL_F(SSLOGGER_FATAL, __VA_ARGS__)

#define SSLN_TRACE_CT(...) \
    if constexpr (SSLN_LEVEL_ENABLED(::ssln::LogLevel::Trace)) SSLN_LOG(SSLOGGER_TRACE, __VA_ARGS__)

#define SSLN_DEBUG_CT(...) \
    if constexpr (SSLN_LEVEL_ENABLED(::ssln::LogLevel::Debug)) SSLN_LOG(SSLOGGER_DEBUG, __VA_ARGS__)

#define SSLN_INFO_CT(...) \
    if constexpr (SSLN_LEVEL_ENABLED(::ssln::LogLevel::Info)) SSLN_LOG(SSLOGGER_INFO, __VA_ARGS__)

#define SSLN_WARN_CT(...) \
    if constexpr (SSLN_LEVEL_ENABLED(::ssln::LogLevel::Warn)) SSLN_LOG(SSLOGGER_WARN, __VA_ARGS__)

#define SSLN_ERROR_CT(...) \
    if constexpr (SSLN_LEVEL_ENABLED(::ssln::LogLevel::Error)) SSLN_LOG(SSLOGGER_ERROR, __VA_ARGS__)

#define SSLN_FATAL_CT(...) \
    if constexpr (SSLN_LEVEL_ENABLED(::ssln::LogLevel::Fatal)) SSLN_LOG(SSLOGGER_FATAL, __VA_ARGS__)

#endif  // SSLN_SSLOGGER_H_
