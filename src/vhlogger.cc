#include "vhlogger/vhlogger.h"
#include <spdlog/async.h>
#include <spdlog/details/log_msg.h>

namespace vgp {

Logger* Logger::GetInstance() {
  static Logger instance;
  return &instance;
}

Logger& Logger::SetFile(const std::string& filename, bool truncate) {
  filename_ = filename;
  file_sink_ = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, truncate);
  UpdateLoggers();
  return *this;
}

Logger& Logger::SetAsyncMode(bool async_mode) {
  async_mode_ = async_mode;
  UpdateLoggers();
  return *this;
}

Logger& Logger::SetLevel(spdlog::level::level_enum level) {
  level_ = level;
  UpdateLoggers();
  return *this;
}

Logger& Logger::SetVerbose(Verbose ver) {
  switch (ver) {
  case Verbose::kLite: pattern_ = "%v"; break;
  case Verbose::kLow: pattern_ = "[%H:%M:%S.%f] %v"; break;
  case Verbose::kMedium: pattern_ = "[%H:%M:%S.%f][%^%L%$][%@] %v"; break;
  case Verbose::kHigh: pattern_ = "[%H:%M:%S.%f][%^%L%$][thread %t][%@] %v"; break;
  case Verbose::kFull: pattern_ = "[%Y-%m-%d %H:%M:%S.%f][%^%L%$][thread %t][%!][%#] %v"; break;
  case Verbose::kUltra: pattern_ = "[%Y-%m-%d %H:%M:%S.%F][%^%L%$][thread %t][%!][%@] %v"; break;
  }
  UpdateLoggers();
  return *this;
}

Logger& Logger::AddLogger(const std::string& name, const std::string& filename, bool daily) {
  if (daily) {
    auto daily_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(filename, 0, 0);
    loggers_[name] = std::make_shared<spdlog::logger>(name, daily_sink);
  } else {
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename);
    loggers_[name] = std::make_shared<spdlog::logger>(name, file_sink);
  }
  UpdateLoggers();
  return *this;
}

void Logger::AddCallback(const CallbackCondition& condition, CallbackFunction callback) {
  callbacks_.push_back({condition, callback});
}

void Logger::LogArray(spdlog::level::level_enum level, const uint8_t* ptr, int size, bool to_file) {
  auto log_msg = fmt::format("Array data: {}", spdlog::to_hex(ptr, ptr + size));
  if (to_file && !loggers_.empty()) {
    loggers_.begin()->second->log(level, log_msg);
  } else if (async_mode_) {
    async_logger_->log(level, log_msg);
  } else {
    console_logger_->log(level, log_msg);
  }
  TriggerCallbacks(level, "", -1, "", log_msg);
}

Logger::Logger():async_mode_(false) {
  console_sink_ = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console_logger_ = std::make_shared<spdlog::logger>("console", console_sink_);
  spdlog::set_default_logger(console_logger_);
  SetupFromEnv();
  UpdateLoggers();
}

void Logger::SetupFromEnv() {
  const char* env_level = std::getenv("SSLN_LOG_LEVEL");
  if (env_level) {
    level_ = ParseLogLevel(env_level);
  } else {
    level_ = VHLOGGER_INFO;
  }
  SetLevel(level_);

  const char* env_verbose = std::getenv("SSLN_LOG_VERBOSE");
  if (env_verbose) {
    int verbose_value = std::atoi(env_verbose);
    switch (verbose_value) {
    case 0:
      verbose_ = Verbose::kLite;
      break;
    case 1:
      verbose_ = Verbose::kLow;
      break;
    case 2:
      verbose_ = Verbose::kMedium;
      break;
    case 3:
      verbose_ = Verbose::kHigh;
      break;
    case 4:
      verbose_ = Verbose::kUltra;
      break;
    default:
      std::cerr << "Invalid Verbose value in SSLN_LOG_VERBOSE: " << verbose_value
        << ". Using default verbose (kLite)." << std::endl;
      verbose_ = Verbose::kLite;
      break;
    }
  } else {
    verbose_ = Verbose::kLite;
  }
  SetVerbose(verbose_);

  const char* env_logfile = std::getenv("SSLN_LOG_FILE");
  if (env_logfile) {
    SetFile(env_logfile);
  }

  const char* env_async = std::getenv("SSLN_LOG_ASYNC");
  if (env_async) {
    SetAsyncMode(std::string(env_async) == "1" || std::string(env_async) == "true");
  }
}

spdlog::level::level_enum Logger::ParseLogLevel(const std::string& level) {
  // Convert to uppercase for case-insensitive comparison
  std::string upper_level = level;
  std::transform(upper_level.begin(), upper_level.end(), upper_level.begin(), ::toupper);

  if (upper_level == "OFF" || upper_level == "6") return VHLOGGER_OFF;
  if (upper_level == "FATAL" || upper_level == "5") return VHLOGGER_FATAL;
  if (upper_level == "ERROR" || upper_level == "4") return VHLOGGER_ERROR;
  if (upper_level == "WARN" || upper_level == "3") return VHLOGGER_WARN;
  if (upper_level == "INFO" || upper_level == "2") return VHLOGGER_INFO;
  if (upper_level == "DEBUG" || upper_level == "1") return VHLOGGER_DEBUG;
  if (upper_level == "TRACE" || upper_level == "0") return VHLOGGER_TRACE;

  // If we get here, the input was invalid. Log a warning and return the default level.
  std::cerr << "Invalid log level: " << level << ". Using default level (INFO)." << std::endl;

  return spdlog::level::info;
}

template<typename LoggerType>
void Logger::UpdateLogger(std::shared_ptr<LoggerType>& logger) {
  if (logger) {
    logger->set_level(level_);
    logger->set_pattern(pattern_);
  }
}

void Logger::UpdateLoggers() {
  if (file_sink_) {
    if (async_mode_) {
      if (!async_logger_) {
        spdlog::init_thread_pool(8192, 1);
        async_logger_ = std::make_shared<spdlog::async_logger>("async_logger", file_sink_, 
          spdlog::thread_pool(), spdlog::async_overflow_policy::block);
        spdlog::register_logger(async_logger_);
      }
      UpdateLogger(async_logger_);
    } else {
      if (!sync_logger_) {
        sync_logger_ = std::make_shared<spdlog::logger>("sync_logger", file_sink_);
        spdlog::register_logger(sync_logger_);
      }
      UpdateLogger(sync_logger_);
    }
  }

  UpdateLogger(console_logger_);

  for (auto& logger_pair : loggers_) {
    UpdateLogger(logger_pair.second);
  }
}

void Logger::TriggerCallbacks(spdlog::level::level_enum level, const char* file, int line,
  const char* func, const std::string& message) {
  spdlog::details::log_msg msg;
  msg.level = level;
  msg.time = std::chrono::system_clock::now();
  msg.source.filename = file;
  msg.source.line = line;
  msg.source.funcname = func;
  msg.payload = message;

  for (const auto& cb : callbacks_) {
    if (MatchesCondition(cb.first, msg)) {
      cb.second(msg);
    }
  }
}

bool Logger::MatchesCondition(const CallbackCondition& condition, const spdlog::details::log_msg& msg) {
  if (condition.level != VHLOGGER_OFF && condition.level != msg.level) {
    return false;
  }
  if (condition.file.has_value() && msg.source.filename != condition.file.value()) {
    return false;
  }
  if (condition.line.has_value() && msg.source.line != condition.line.value()) {
    return false;
  }
  if (condition.function.has_value() && msg.source.funcname != condition.function.value()) {
    return false;
  }
  if (condition.message.has_value() && std::string(msg.payload.data(), msg.payload.size()).find(condition.message.value()) == std::string::npos) {
    return false;
  }
  return true;
}


} // namespace vgp

