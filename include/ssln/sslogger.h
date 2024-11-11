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

// Define logging level macros
#define SSLOGGER_TRACE    spdlog::level::trace
#define SSLOGGER_DEBUG    spdlog::level::debug
#define SSLOGGER_INFO     spdlog::level::info
#define SSLOGGER_WARN     spdlog::level::warn
#define SSLOGGER_ERROR    spdlog::level::err
#define SSLOGGER_FATAL    spdlog::level::critical
#define SSLOGGER_OFF      spdlog::level::off

namespace ssln {

/*!
 * @brief Enumeration of logging levels for compile-time checks
 */
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

/*!
 * @brief Template for compile-time log level comparison
 */
template<LogLevel A, LogLevel B>
constexpr bool LogLevelLE() {
    return static_cast<int>(A) <= static_cast<int>(B);
}

#define SSLN_LEVEL_ENABLED(level) \
    ::ssln::LogLevelLE<SSLN_ACTIVE_LEVEL, level>()

/*!
 * @brief Converts LogLevel enum to spdlog level
 */
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

/*!
 * @brief Main logger class providing logging functionality
 */
class Logger {
public:
    /*!
     * @brief Verbosity levels for log formatting
     */
    enum class Verbose { 
        kLite,    ///< Minimal logging format
        kLow,     ///< Basic time and message
        kMedium,  ///< Standard logging format
        kHigh,    ///< Detailed logging with thread info
        kFull,    ///< Full logging with source info
        kUltra    ///< Maximum verbosity
    };

    using CallbackFunction = std::function<void(const spdlog::details::log_msg&)>;

    /*!
     * @brief Conditions for triggering log callbacks
     */
    struct CallbackCondition {
        spdlog::level::level_enum level = SSLOGGER_OFF;
        tl::optional<std::string> file;
        tl::optional<int> line;
        tl::optional<std::string> function;
        tl::optional<std::string> message;
    };

    // Constructor and Destructor
    Logger();
    ~Logger();

    // Initialization
    static void Init(const std::string& log_dir = ".", 
                    const std::string& log_file = "",
                    bool async_mode = false,
                    spdlog::level::level_enum level = SSLOGGER_INFO,
                    Verbose verbose = Verbose::kMedium,
                    bool allow_env_override = true);

    // Configuration methods
    Logger& SetFile(const std::string& filename, bool truncate = true);
    Logger& SetAsyncMode(bool async_mode);
    Logger& SetLevel(spdlog::level::level_enum level);
    Logger& SetVerbose(Verbose ver);
    Logger& AddLogger(const std::string& name, const std::string& filename, bool daily = false);

    // Async configuration methods
    Logger& SetAsyncQueueSize(size_t size);
    Logger& SetWorkerThreads(int threads);
    Logger& SetFlushInterval(std::chrono::seconds interval);

    // Callback management
    void AddCallback(const CallbackCondition& condition, CallbackFunction callback);

    template<typename... Args>
    using format_string_t = fmt::format_string<Args...>;

    template<typename... Args>
    void Log(spdlog::level::level_enum level, const char* file, int line, const char* func, format_string_t<Args...> fmt, const Args&... args) {
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
    void LogF(spdlog::level::level_enum level, const char* file, int line,
              const char* func, format_string_t<Args...> fmt, const Args&... args) {
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
    void LogTo(const std::string& logger_name, spdlog::level::level_enum level, const char* file, int line, const char* func, format_string_t<Args...> fmt, const Args&... args) {
        auto it = loggers_.find(logger_name);
        if (it != loggers_.end()) {
            it->second->log(spdlog::source_loc{file, line, func}, level, fmt, args...);
        }
#ifdef SSLOGGER_ENABLE_CB
        TriggerCallbacks(level, file, line, func, fmt::format(fmt, args...));
#endif
    }

    // add batch log
    template<typename Range>
    void LogBatch(spdlog::level::level_enum level, const Range& range) {
        if (config_.async_mode && async_logger_) {
            async_logger_->log(level, fmt::join(range, ", "));
        }
    }

    void LogArray(spdlog::level::level_enum level, const uint8_t* ptr, 
                 int size, bool to_file = false);

private:
    // Configuration structure
    struct Config {
        std::string log_dir = ".";
        std::string log_file = "uv.log";
        bool async_mode = false;
        spdlog::level::level_enum level = SSLOGGER_INFO;
        Verbose verbose = Verbose::kMedium;
        std::string pattern_ = "[%Y-%m-%d %H:%M:%S.%f][%^%L%$][%t][%@] %v";
        bool allow_env_override = true;

        size_t async_queue_size = 8192;
        int worker_threads = 1;
        size_t buffer_size = 8192;
        std::chrono::seconds flush_interval{3};
    };

    // Internal methods
    void LoadFromEnv();
    template<typename LoggerType>
    void UpdateLogger(std::shared_ptr<LoggerType>& logger);
    void UpdateLoggers();
    void UpdateLoggersInternal();
    void TriggerCallbacks(spdlog::level::level_enum level, const char* file, int line,
                         const char* func, const std::string& message);
    bool MatchesCondition(const CallbackCondition& condition, 
                         const spdlog::details::log_msg& msg);

    // Member variables
    Config config_;
    mutable std::mutex config_mutex_;
    std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> console_sink_;
    std::shared_ptr<spdlog::sinks::basic_file_sink_mt> file_sink_;
    std::shared_ptr<spdlog::logger> console_logger_;
    std::shared_ptr<spdlog::logger> sync_logger_;
    std::shared_ptr<spdlog::async_logger> async_logger_;
    std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> loggers_;

    static constexpr size_t kMaxCallbacks = 16;
    std::array<std::pair<CallbackCondition, CallbackFunction>, kMaxCallbacks> callbacks_;
    size_t callback_count_ = 0;
};

// Global logger instance
extern std::shared_ptr<Logger> g_logger;

} // namespace ssln

// Logging macros
#define SSLN_LOG_ARRAY(level, ptr, size) ssln::g_logger->LogArray(level, ptr, size, false)
#define SSLN_LOGF_ARRAY(level, ptr, size) ssln::g_logger->LogArray(level, ptr, size, true)

// Basic logging macros
#define SSLN_LOG(level, ...) \
    if(ssln::g_logger) ssln::g_logger->Log(level, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define SSLN_TRACE(...) SSLN_LOG(SSLOGGER_TRACE, __VA_ARGS__)
#define SSLN_DEBUG(...) SSLN_LOG(SSLOGGER_DEBUG, __VA_ARGS__)
#define SSLN_INFO(...)  SSLN_LOG(SSLOGGER_INFO, __VA_ARGS__)
#define SSLN_WARN(...)  SSLN_LOG(SSLOGGER_WARN, __VA_ARGS__)
#define SSLN_ERROR(...) SSLN_LOG(SSLOGGER_ERROR, __VA_ARGS__)
#define SSLN_FATAL(...) SSLN_LOG(SSLOGGER_FATAL, __VA_ARGS__)

// File logging macros
#define SSLN_LOG_TO(logger_name, level, ...) \
    if (ssln::g_logger) ssln::g_logger->LogTo(logger_name, level, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define SSLN_TRACE_TO(logger_name, ...) SSLN_LOG_TO(logger_name, SSLOGGER_TRACE, __VA_ARGS__)
#define SSLN_DEBUG_TO(logger_name, ...) SSLN_LOG_TO(logger_name, SSLOGGER_DEBUG, __VA_ARGS__)
#define SSLN_INFO_TO(logger_name, ...)  SSLN_LOG_TO(logger_name, SSLOGGER_INFO, __VA_ARGS__)
#define SSLN_WARN_TO(logger_name, ...)  SSLN_LOG_TO(logger_name, SSLOGGER_WARN, __VA_ARGS__)
#define SSLN_ERROR_TO(logger_name, ...) SSLN_LOG_TO(logger_name, SSLOGGER_ERROR, __VA_ARGS__)
#define SSLN_FATAL_TO(logger_name, ...) SSLN_LOG_TO(logger_name, SSLOGGER_FATAL, __VA_ARGS__)

// File-specific logging macros
#define SSLN_LOG_F(level, ...) \
    if (ssln::g_logger) ssln::g_logger->LogF(level, __FILE__, __LINE__, __func__, __VA_ARGS__)

#define SSLN_TRACEF(...) SSLN_LOG_F(SSLOGGER_TRACE, __VA_ARGS__)
#define SSLN_DEBUGF(...) SSLN_LOG_F(SSLOGGER_DEBUG, __VA_ARGS__)
#define SSLN_INFOF(...)  SSLN_LOG_F(SSLOGGER_INFO, __VA_ARGS__)
#define SSLN_WARNF(...)  SSLN_LOG_F(SSLOGGER_WARN, __VA_ARGS__)
#define SSLN_ERRORF(...) SSLN_LOG_F(SSLOGGER_ERROR, __VA_ARGS__)
#define SSLN_FATALF(...) SSLN_LOG_F(SSLOGGER_FATAL, __VA_ARGS__)

// Compile-time logging macros
#define SSLN_LOGGER_CALL(level, ...) \
    if constexpr (SSLN_LEVEL_ENABLED(level)) SSLN_LOG(ToSpdLogLevel(level), __VA_ARGS__)

#define SSLN_LOGGER_CALL_F(level, ...) \
    if constexpr (SSLN_LEVEL_ENABLED(level)) SSLN_LOG_F(ToSpdLogLevel(level), __VA_ARGS__)

// Compile-time level checks
#define SSLN_TRACE_ENABLED() SSLN_LEVEL_ENABLED(::ssln::LogLevel::Trace)
#define SSLN_DEBUG_ENABLED() SSLN_LEVEL_ENABLED(::ssln::LogLevel::Debug)
#define SSLN_INFO_ENABLED()  SSLN_LEVEL_ENABLED(::ssln::LogLevel::Info)
#define SSLN_WARN_ENABLED()  SSLN_LEVEL_ENABLED(::ssln::LogLevel::Warn)
#define SSLN_ERROR_ENABLED() SSLN_LEVEL_ENABLED(::ssln::LogLevel::Error)
#define SSLN_FATAL_ENABLED() SSLN_LEVEL_ENABLED(::ssln::LogLevel::Fatal)

// Compile-time logging macros
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

// Debug assertions
#ifdef SSLN_DEBUG_BUILD
    #define SSLN_ASSERT(condition, message) \
        do { \
            if (!(condition)) { \
                SSLN_FATAL("Assertion failed: {}", message); \
                std::abort(); \
            } \
        } while(0)
#else
    #define SSLN_ASSERT(condition, message) do {} while(0)
#endif

#endif  // SSLN_SSLOGGER_H_
