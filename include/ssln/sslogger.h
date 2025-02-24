#ifndef SSLN_SSLOGGER_H_
#define SSLN_SSLOGGER_H_

#include "quill/Logger.h"
#include <string>

// Disable Quill's non-prefixed macros to avoid conflicts
#define QUILL_DISABLE_NON_PREFIXED_MACROS
#include "quill/LogMacros.h"

namespace ssln {

// Verbosity presets for different logging formats
enum class Verbose {
  kLite,    // Message only
  kLow,     // Time + message
  kMedium,  // Time + level + location
  kHigh,    // Time + level + thread + location
  kFull,    // Full time + level + thread + function + line
  kUltra    // Most detailed format with nanosecond precision
};

// 预定义的 Logger 实例，供库内部使用
extern quill::Logger* hybrid_logger;     // hybrid.log
extern quill::Logger* axi_master_logger;  // axi_master.log
extern quill::Logger* axi_slave_logger;   // axi_slave.log
extern quill::Logger* perf_logger;        // perf.log
extern quill::Logger* console_logger;     // 控制台输出
extern quill::Logger* daily_logger;       // daily_<date>.log

// 默认 Logger，用户可通过环境变量调整
extern quill::Logger* default_logger;

// 日志宏，供库内部显式选择 Logger
#define SSLN_LOG_TRACE_L3(logger, fmt, ...)  QUILL_LOG_TRACE_L3(logger, fmt, ##__VA_ARGS__)
#define SSLN_LOG_TRACE_L2(logger, fmt, ...)  QUILL_LOG_TRACE_L2(logger, fmt, ##__VA_ARGS__)
#define SSLN_LOG_TRACE_L1(logger, fmt, ...)  QUILL_LOG_TRACE_L1(logger, fmt, ##__VA_ARGS__)
#define SSLN_LOG_DEBUG(logger, fmt, ...)     QUILL_LOG_DEBUG(logger, fmt, ##__VA_ARGS__)
#define SSLN_LOG_INFO(logger, fmt, ...)      QUILL_LOG_INFO(logger, fmt, ##__VA_ARGS__)
#define SSLN_LOG_WARNING(logger, fmt, ...)   QUILL_LOG_WARNING(logger, fmt, ##__VA_ARGS__)
#define SSLN_LOG_ERROR(logger, fmt, ...)     QUILL_LOG_ERROR(logger, fmt, ##__VA_ARGS__)
#define SSLN_LOG_CRITICAL(logger, fmt, ...)  QUILL_LOG_CRITICAL(logger, fmt, ##__VA_ARGS__)

// 简化的默认 Logger 宏，供快速使用
#define SSLN_TRACE_L3(fmt, ...)  QUILL_LOG_TRACE_L3(::ssln::default_logger, fmt, ##__VA_ARGS__)
#define SSLN_TRACE_L2(fmt, ...)  QUILL_LOG_TRACE_L2(::ssln::default_logger, fmt, ##__VA_ARGS__)
#define SSLN_TRACE_L1(fmt, ...)  QUILL_LOG_TRACE_L1(::ssln::default_logger, fmt, ##__VA_ARGS__)
#define SSLN_DEBUG(fmt, ...)     QUILL_LOG_DEBUG(::ssln::default_logger, fmt, ##__VA_ARGS__)
#define SSLN_INFO(fmt, ...)      QUILL_LOG_INFO(::ssln::default_logger, fmt, ##__VA_ARGS__)
#define SSLN_WARNING(fmt, ...)   QUILL_LOG_WARNING(::ssln::default_logger, fmt, ##__VA_ARGS__)
#define SSLN_ERROR(fmt, ...)     QUILL_LOG_ERROR(::ssln::default_logger, fmt, ##__VA_ARGS__)
#define SSLN_CRITICAL(fmt, ...)  QUILL_LOG_CRITICAL(::ssln::default_logger, fmt, ##__VA_ARGS__)

// 可选短别名
#ifdef SSLN_SHORT_MACROS
#define TRACE3(fmt, ...)  SSLN_TRACE_L3(fmt, ##__VA_ARGS__)
#define TRACE2(fmt, ...)  SSLN_TRACE_L2(fmt, ##__VA_ARGS__)
#define TRACE1(fmt, ...)  SSLN_TRACE_L1(fmt, ##__VA_ARGS__)
#define DEBUG(fmt, ...)   SSLN_DEBUG(fmt, ##__VA_ARGS__)
#define INFO(fmt, ...)    SSLN_INFO(fmt, ##__VA_ARGS__)
#define WARN(fmt, ...)    SSLN_WARNING(fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...)   SSLN_ERROR(fmt, ##__VA_ARGS__)
#define FATAL(fmt, ...)   SSLN_CRITICAL(fmt, ##__VA_ARGS__)
#endif


std::string GetPattern(Verbose ver);

namespace detail {
    // 获取 logger 对应的文件路径
    std::string GetLoggerFilePath(const std::string& logger_name);
} // namespace detail


// Logger setup functions
quill::Logger* SetupConsoleLogger(const std::string& logger_name, 
                                   Verbose verbose = Verbose::kLite, 
                                   quill::LogLevel level = quill::LogLevel::Info);

quill::Logger* SetupFileLogger(const char* log_file, const std::string& logger_name, 
                                Verbose verbose = Verbose::kMedium, 
                                quill::LogLevel level = quill::LogLevel::Info, 
                                bool append_date = false);

quill::Logger* SetupRotatingLogger(const char* log_file, size_t max_file_size, size_t max_files,
                                    const std::string& logger_name, 
                                    Verbose verbose = Verbose::kMedium, 
                                    quill::LogLevel level = quill::LogLevel::Info);

quill::Logger* SetupPerfLogger(const char* log_file, const std::string& logger_name,
                                    quill::LogLevel level = quill::LogLevel::Info, 
                                    Verbose verbose = Verbose::kLite);

quill::Logger* get_logger(const std::string& name);
quill::Logger* set_default_logger(const std::string& name);
quill::Logger* set_default_logger(quill::Logger* logger);

} // namespace ssln

#endif // SSLN_SSLOGGER_H_
