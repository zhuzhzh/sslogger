#ifndef SSLN_SSLOGGER_H_
#define SSLN_SSLOGGER_H_

#include "quill/Logger.h"

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

namespace detail {

// Gets environment variable value or returns default if not set
std::string GetEnvOr(const char* name, const std::string& default_value);

// Gets the pattern string for the specified verbosity level
std::string GetPattern(Verbose ver);

// Create pattern formatter options based on verbosity
quill::PatternFormatterOptions GetFormatterOptions(Verbose verbose);

} // namespace detail

// Default logger instance
extern quill::Logger* default_logger;
extern quill::Logger* console_logger;
extern quill::Logger* file_logger;
extern quill::Logger* rotating_logger;
extern quill::Logger* perf_logger;

// Set the default logger
void set_default_logger(quill::Logger* logger);

// Get logger by name
quill::Logger* get_logger(const std::string& name);

// Initialize backend if not already running
void InitBackend();

// Setup functions
quill::Logger* SetupConsole(quill::LogLevel level = quill::LogLevel::Info,
                         Verbose verbose = Verbose::kLite,
                         const std::string& logger_name = "console_logger");

quill::Logger* SetupFile(const char* log_file,
                      quill::LogLevel level = quill::LogLevel::Info,
                      Verbose verbose = Verbose::kMedium,
                      const std::string& logger_name = "file_logger",
                      bool append_date = false);

quill::Logger* SetupRotatingFile(const char* log_file,
                              size_t max_file_size,
                              size_t max_files,
                              quill::LogLevel level = quill::LogLevel::Info,
                              Verbose verbose = Verbose::kMedium,
                              const std::string& logger_name = "rotating_logger",
                              bool append_date = false);

quill::Logger* SetupPerfFile(const char* log_file,
                      quill::LogLevel level = quill::LogLevel::Info,
                      Verbose verbose = Verbose::kLite,
                      const std::string& logger_name = "perf_logger");

}  // namespace ssln

#endif  // SSLN_SSLOGGER_H_