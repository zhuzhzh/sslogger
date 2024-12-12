#ifndef SSLN_SSLOGGER_H_
#define SSLN_SSLOGGER_H_

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/stopwatch.h>
#include <spdlog/fmt/bin_to_hex.h>
#include <cstdlib>

namespace ssln {

// Verbosity presets for different logging formats.
enum class Verbose {
  kLite,    // Message only
  kLow,     // Time + message
  kMedium,  // Time + level + location
  kHigh,    // Time + level + thread + location
  kFull,    // Full time + level + thread + function + line
  kUltra    // Most detailed format
};

namespace detail {

// Gets the pattern string for the specified verbosity level.
inline std::string GetPattern(Verbose ver) {
  switch (ver) {
    case Verbose::kLite:   return "%v";
    case Verbose::kLow:    return "[%H:%M:%S.%f] %v";
    case Verbose::kMedium: return "[%H:%M:%S.%f][%^%L%$][%@] %v";
    case Verbose::kHigh:   return "[%H:%M:%S.%f][%^%L%$][thread %t][%@] %v";
    case Verbose::kFull:   return "[%Y-%m-%d %H:%M:%S.%f][%^%L%$][thread %t][%!][%#] %v";
    case Verbose::kUltra:  return "[%Y-%m-%d %H:%M:%S.%F][%^%L%$][thread %t][%!][%@] %v";
    default:               return "[%Y-%m-%d %H:%M:%S.%f][%^%L%$][%t] %v";
  }
}

// Gets environment variable value or returns default if not set.
inline std::string GetEnvOr(const char* name, const std::string& default_value) {
  const char* val = std::getenv(name);
  return val ? val : default_value;
}

// Gets log level from environment variable or returns default.
inline spdlog::level::level_enum GetLevelFromEnv(
    spdlog::level::level_enum default_level) {
  const char* level = std::getenv("SSLN_LOG_LEVEL");
  if (!level) return default_level;
  
  std::string level_str = level;
  if (level_str == "trace") return spdlog::level::trace;
  if (level_str == "debug") return spdlog::level::debug;
  if (level_str == "info")  return spdlog::level::info;
  if (level_str == "warn")  return spdlog::level::warn;
  if (level_str == "error") return spdlog::level::err;
  if (level_str == "fatal") return spdlog::level::critical;
  if (level_str == "off")   return spdlog::level::off;
  return default_level;
}

// Gets verbosity level from environment variable or returns default.
inline Verbose GetVerboseFromEnv(Verbose default_verbose) {
  const char* verb = std::getenv("SSLN_VERBOSITY");
  if (!verb) return default_verbose;

  std::string verb_str = verb;
  if (verb_str == "lite")   return Verbose::kLite;
  if (verb_str == "low")    return Verbose::kLow;
  if (verb_str == "medium") return Verbose::kMedium;
  if (verb_str == "high")   return Verbose::kHigh;
  if (verb_str == "full")   return Verbose::kFull;
  if (verb_str == "ultra")  return Verbose::kUltra;
  return default_verbose;
}

// Thread pool initialization flag.
inline bool& ThreadPoolInitialized() {
  static bool initialized = false;
  return initialized;
}

// Initializes thread pool if not already initialized.
inline void InitThreadPoolOnce() {
  if (!ThreadPoolInitialized()) {
    const auto queue_size = std::stoi(GetEnvOr("SSLN_QUEUE_SIZE", "8192"));
    const auto n_threads = std::stoi(GetEnvOr("SSLN_THREADS", "1"));
    spdlog::init_thread_pool(queue_size, n_threads);
    ThreadPoolInitialized() = true;
  }
}

}  // namespace detail

// Initializes console logger with specified settings.
inline void InitConsole(
    spdlog::level::level_enum level = spdlog::level::info,
    Verbose verbose = Verbose::kMedium,
    const std::string& logger_name = "console") {
  try {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto logger = std::make_shared<spdlog::logger>(logger_name, console_sink);
    
    logger->set_level(detail::GetLevelFromEnv(level));
    logger->set_pattern(detail::GetPattern(detail::GetVerboseFromEnv(verbose)));
    
    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
  } catch (const spdlog::spdlog_ex& ex) {
    throw std::runtime_error(std::string("Logger initialization failed: ") + ex.what());
  }
}

// Initializes synchronous file logger with specified settings.
inline void InitSyncFile(
    const std::string& log_dir,
    const std::string& log_name,
    spdlog::level::level_enum level = spdlog::level::info,
    Verbose verbose = Verbose::kMedium,
    const std::string& logger_name = "sync_logger") {
  try {
    // Use environment variables if set
    const auto final_dir = detail::GetEnvOr("SSLN_LOG_DIR", log_dir);
    const auto final_name = detail::GetEnvOr("SSLN_LOG_NAME", log_name);
    
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        final_dir + "/" + final_name);
        
    auto logger = std::make_shared<spdlog::logger>(logger_name, file_sink);
        
    logger->set_level(detail::GetLevelFromEnv(level));
    logger->set_pattern(detail::GetPattern(detail::GetVerboseFromEnv(verbose)));
    
    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
  } catch (const spdlog::spdlog_ex& ex) {
    throw std::runtime_error(std::string("Logger initialization failed: ") + ex.what());
  }
}

// Initializes asynchronous file logger with specified settings.
inline void InitAsyncFile(
    const std::string& log_dir,
    const std::string& log_name,
    spdlog::level::level_enum level = spdlog::level::info,
    Verbose verbose = Verbose::kMedium, 
    const std::string& logger_name = "async_logger") {
  try {
    // Use environment variables if set
    const auto final_dir = detail::GetEnvOr("SSLN_LOG_DIR", log_dir);
    const auto final_name = detail::GetEnvOr("SSLN_LOG_NAME", log_name);
    const auto flush_secs = std::stoi(detail::GetEnvOr("SSLN_FLUSH_SECS", "3"));
    
    detail::InitThreadPoolOnce();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        final_dir + "/" + final_name);
        
    auto logger = std::make_shared<spdlog::async_logger>(
        logger_name, file_sink, spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);
        
    logger->set_level(detail::GetLevelFromEnv(level));
    logger->set_pattern(detail::GetPattern(detail::GetVerboseFromEnv(verbose)));
    
    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
    spdlog::flush_every(std::chrono::seconds(flush_secs));
  } catch (const spdlog::spdlog_ex& ex) {
    throw std::runtime_error(std::string("Logger initialization failed: ") + ex.what());
  }
}

// Initializes rotating file logger with specified settings.
inline void InitRotatingFile(
    const std::string& log_dir,
    const std::string& log_name,
    size_t max_file_size,
    size_t max_files,
    spdlog::level::level_enum level = spdlog::level::info,
    Verbose verbose = Verbose::kMedium,
    const std::string& logger_name = "rotating_logger") {
  try {
    // Use environment variables if set
    const auto final_dir = detail::GetEnvOr("SSLN_LOG_DIR", log_dir);
    const auto final_name = detail::GetEnvOr("SSLN_LOG_NAME", log_name);
    const auto final_max_size = std::stoull(detail::GetEnvOr(
        "SSLN_MAX_FILE_SIZE", std::to_string(max_file_size)));
    const auto final_max_files = std::stoull(detail::GetEnvOr(
        "SSLN_MAX_FILES", std::to_string(max_files)));
    
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        final_dir + "/" + final_name,
        final_max_size,
        final_max_files);
        
    auto logger = std::make_shared<spdlog::logger>(logger_name, file_sink);
        
    logger->set_level(detail::GetLevelFromEnv(level));
    logger->set_pattern(detail::GetPattern(detail::GetVerboseFromEnv(verbose)));
    
    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
  } catch (const spdlog::spdlog_ex& ex) {
    throw std::runtime_error(std::string("Logger initialization failed: ") + ex.what());
  }
}

// Alias for spdlog::stopwatch
using Stopwatch = spdlog::stopwatch;

}  // namespace ssln

#endif  // SSLN_SSLOGGER_H_