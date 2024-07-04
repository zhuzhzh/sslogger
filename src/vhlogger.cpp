#include "vhlogger/vhlogger.hpp"
#include <ctime>
#include <cstdlib>

namespace vgp {

Logger& Logger::GetInstance() {
  static Logger instance;
  return instance;
}

Logger::Logger() {
  const char* env_level = std::getenv("UV_HYBRID_VERBOSE");
  if (env_level) {
    int level = std::atoi(env_level);
    log_level_ = static_cast<LogLevel>(level);
  }
}

Logger::~Logger() {
  if (log_file_.is_open()) {
    log_file_.close();
  }
}

void Logger::SetFormat(Format format) {
  format_ = format;
}

void Logger::SetLogFile(const std::string& filename, bool append) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (log_file_.is_open()) {
    log_file_.close();
  }
  log_file_.open(filename, append ? std::ios_base::app : std::ios_base::trunc);
}

void Logger::SetLogLevel(LogLevel level) {
  log_level_ = level;
}

std::string Logger::FormatHeader(LogLevel level, const char* file, int line) {
  std::string header;
  
  if (format_ == Format::FULL || format_ == Format::MEDIUM) {
    auto now = std::chrono::system_clock::now();
    header += fmt::format("[{:%Y-%m-%d %H:%M:%S}] ", now);
  }

  if (format_ == Format::FULL) {
    const char* level_str;
    switch (level) {
      case LogLevel::DEBUG: level_str = "DEBUG"; break;
      case LogLevel::INFO:  level_str = "INFO "; break;
      case LogLevel::WARN:  level_str = "WARN "; break;
      case LogLevel::ERROR: level_str = "ERROR"; break;
      case LogLevel::FATAL: level_str = "FATAL"; break;
    }
    header += fmt::format("[{}] [{}:{}] ", level_str, file, line);
  }

  return header;
}

void Logger::WriteLog(const std::string& message) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (log_file_.is_open()) {
    log_file_ << message << std::endl;
  } else {
    std::cout << message << std::endl;
  }
}

}  // namespace vgp
