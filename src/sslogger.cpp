#include "ssln/sslogger.h"
#include <iostream>
#include "quill/Backend.h"
#include "quill/Frontend.h"
#include "quill/Logger.h"
#include "quill/sinks/ConsoleSink.h"
#include "quill/sinks/FileSink.h"
#include "quill/sinks/RotatingFileSink.h"

namespace ssln {

// Define global logger instances (not just declare)
quill::Logger* default_logger = nullptr;
quill::Logger* console_logger = nullptr;
quill::Logger* file_logger = nullptr;
quill::Logger* rotating_logger = nullptr;
quill::Logger* perf_logger = nullptr;

void set_default_logger(quill::Logger* logger) {
  default_logger = logger;
}

quill::Logger* get_logger(const std::string& name) {
  return quill::Frontend::get_logger(name);
}

namespace detail {

// Gets environment variable value or returns default if not set
inline std::string GetEnvOr(const char* name, const std::string& default_value) {
  const char* val = std::getenv(name);
  return val ? val : default_value;
}

// Gets log level from environment variable or returns default
inline quill::LogLevel GetLevelFromEnv(quill::LogLevel default_level) {
  const char* level = std::getenv("SSLN_LOG_LEVEL");
  if (!level) return default_level;
  
  std::string level_str = level;
  if (level_str == "trace") return quill::LogLevel::TraceL3;
  if (level_str == "debug") return quill::LogLevel::Debug;
  if (level_str == "info")  return quill::LogLevel::Info;
  if (level_str == "warn")  return quill::LogLevel::Warning;
  if (level_str == "error") return quill::LogLevel::Error;
  if (level_str == "critical") return quill::LogLevel::Critical;
  if (level_str == "off")   return quill::LogLevel::None;
  return default_level;
}

// Gets verbosity level from environment variable or returns default
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

// Gets the pattern string for the specified verbosity level
std::string GetPattern(Verbose ver) {
  switch (ver) {
    case Verbose::kLite:   return "%(message)";
    case Verbose::kLow:    return "[%(time)] %(message)";
    case Verbose::kMedium: return "[%(time)] [%(log_level)] [%(file_name):%(line_number)] %(message)";
    case Verbose::kHigh:   return "[%(time)] [%(log_level)] [%(thread_id)] [%(file_name):%(line_number)] %(message)";
    case Verbose::kFull:   return "[%(time)] [%(log_level)] [%(thread_id)] [%(caller_function)] [%(file_name):%(line_number)] %(message)";
    case Verbose::kUltra:  return "[%(time)] [%(log_level)] [%(thread_id)] [%(caller_function)] [%(file_name):%(line_number)] %(message)";
    default:               return "[%(time)] [%(log_level)] [%(thread_id)] %(message)";
  }
}

// Create pattern formatter options based on verbosity
quill::PatternFormatterOptions GetFormatterOptions(Verbose verbose) {
  return quill::PatternFormatterOptions{
    GetPattern(GetVerboseFromEnv(verbose)),
    "%H:%M:%S.%Qns",
    quill::Timezone::LocalTime
  };
}

// Gets file path from environment variables or returns default
inline std::string GetLogFilePath(const char* log_file) {
  std::string dir = GetEnvOr("SSLN_LOG_DIR", "");
  std::string name = GetEnvOr("SSLN_LOG_NAME", log_file);
  return dir.empty() ? name : dir + "/" + name;
}

// Gets max file size from environment variable or returns default
inline size_t GetMaxFileSizeFromEnv(size_t default_size) {
  try {
    std::string size_str = GetEnvOr("SSLN_MAX_FILE_SIZE", std::to_string(default_size));
    return std::stoull(size_str);
  } catch (...) {
    return default_size;
  }
}

// Gets max files from environment variable or returns default
inline size_t GetMaxFilesFromEnv(size_t default_files) {
  try {
    std::string files_str = GetEnvOr("SSLN_MAX_FILES", std::to_string(default_files));
    return std::stoull(files_str);
  } catch (...) {
    return default_files;
  }
}

} // namespace detail

// Initialize backend if not already running
void InitBackend() {
  if (!quill::Backend::is_running()) {
    // Start the backend thread
    quill::Backend::start();
  }
}

quill::Logger* SetupConsole(quill::LogLevel level, Verbose verbose, const std::string& logger_name) {
  try {
    InitBackend();
    
    // Create console sink
    auto console_sink = quill::Frontend::create_or_get_sink<quill::ConsoleSink>("console_sink");
    
    // Create and configure logger
    auto logger = quill::Frontend::create_or_get_logger(
        logger_name, std::move(console_sink), detail::GetFormatterOptions(verbose));
    logger->set_log_level(detail::GetLevelFromEnv(level));
    
    // Set as default if no default logger exists
    if (!default_logger) {
      default_logger = logger;
    }

    if (logger_name == "console_logger") {
      console_logger = logger;
    }
    
    return logger;
  } catch (const std::exception& ex) {
    throw std::runtime_error(std::string("Logger initialization failed: ") + ex.what());
  }
}

quill::Logger* SetupFile(const char* log_file, quill::LogLevel level, Verbose verbose,
                      const std::string& logger_name, bool append_date) {
  try {
    InitBackend();
    
    // Get file path from environment variables
    std::string file_path = detail::GetLogFilePath(log_file);
    
    // Create file sink with configuration
    auto file_sink = quill::Frontend::create_or_get_sink<quill::FileSink>(
        file_path.c_str(),
        [append_date]() {
          quill::FileSinkConfig cfg;
          cfg.set_open_mode('w');
          if (append_date) {
            cfg.set_filename_append_option(quill::FilenameAppendOption::StartDateTime);
          } else {
            cfg.set_filename_append_option(quill::FilenameAppendOption::None);
          }
          return cfg;
        }(),
        quill::FileEventNotifier{});
    
    // Create and configure logger
    auto logger = quill::Frontend::create_or_get_logger(
        logger_name, std::move(file_sink), detail::GetFormatterOptions(verbose));

    auto log_level = detail::GetLevelFromEnv(level);
    logger->set_log_level(log_level);
    
    // Set as default if no default logger exists
    if (!default_logger) {
      default_logger = logger;
    }

    if (logger_name == "file_logger") {
      file_logger = logger;
    }
    
    return logger;
  } catch (const std::exception& ex) {
    throw std::runtime_error(std::string("Logger initialization failed: ") + ex.what());
  }
}

quill::Logger* SetupRotatingFile(const char* log_file, size_t max_file_size, size_t max_files,
                              quill::LogLevel level, Verbose verbose, const std::string& logger_name,
                              bool append_date) {
  try {
    InitBackend();
    
    // Get configuration from environment variables
    std::string file_path = detail::GetLogFilePath(log_file);
    size_t final_max_size = detail::GetMaxFileSizeFromEnv(max_file_size);
    size_t final_max_files = detail::GetMaxFilesFromEnv(max_files);
    
    // Create rotating file sink with configuration
    auto rotating_sink = quill::Frontend::create_or_get_sink<quill::RotatingFileSink>(
        file_path.c_str(),
        [final_max_size, final_max_files, append_date]() {
          quill::RotatingFileSinkConfig cfg;
          cfg.set_open_mode('w');
          if (append_date) {
            cfg.set_filename_append_option(quill::FilenameAppendOption::StartDateTime);
          } else {
            cfg.set_filename_append_option(quill::FilenameAppendOption::None);
          }
          cfg.set_rotation_max_file_size(final_max_size);
          cfg.set_max_backup_files(final_max_files);
          return cfg;
        }());
    
    // Create and configure logger
    auto logger = quill::Frontend::create_or_get_logger(
        logger_name, std::move(rotating_sink), detail::GetFormatterOptions(verbose));
    logger->set_log_level(detail::GetLevelFromEnv(level));
    std::cout << "Rotating logger log level: " << static_cast<int>(logger->get_log_level()) << std::endl;

    // Set as default if no default logger exists
    if (!default_logger) {
      default_logger = logger;
    }

    if (logger_name == "rotating_logger") {
      rotating_logger = logger;
    }
    
    return logger;
  } catch (const std::exception& ex) {
    throw std::runtime_error(std::string("Logger initialization failed: ") + ex.what());
  }
}

quill::Logger* SetupPerfFile(const char* log_file, quill::LogLevel level, Verbose verbose, 
                      const std::string& logger_name) {
  try {
    InitBackend();

    auto perf_sink = quill::Frontend::create_or_get_sink<quill::FileSink>(
        log_file,
        []() {
          quill::FileSinkConfig cfg;
          cfg.set_open_mode('w');
          return cfg;
        }(),
        quill::FileEventNotifier{});

    auto logger = quill::Frontend::create_or_get_logger(
        logger_name, std::move(perf_sink), quill::PatternFormatterOptions{"%(message)"});
    logger->set_log_level(detail::GetLevelFromEnv(level));

    if (logger_name == "perf_logger") {
      perf_logger = logger;
    }
    
    return logger;
  } catch (const std::exception& ex) {
    throw std::runtime_error(std::string("Logger initialization failed: ") + ex.what());
  }
}

} // namespace ssln 
