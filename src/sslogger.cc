#include "ssln/sslogger.h"
#include <spdlog/async.h>
#include <spdlog/details/log_msg.h>
#include <iostream>

namespace ssln {

ssln::Logger* g_logger = nullptr;

// 添加析构函数的实现
Logger::~Logger() {
    // 清理所有日志器
    loggers_.clear();
    async_logger_.reset();
    sync_logger_.reset();
    console_logger_.reset();
    file_sink_.reset();
    console_sink_.reset();
}

// 添加构造函数的实现（如果还没有）
Logger::Logger() {
    // 初始化控制台日志器
    console_sink_ = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_logger_ = std::make_shared<spdlog::logger>("console", console_sink_);
    spdlog::register_logger(console_logger_);
}

void Logger::Init(const std::string& log_dir,
                 const std::string& log_file,
                 bool async_mode,
                 spdlog::level::level_enum level,
                 Verbose verbose,
                 bool allow_env_override) {
    if (g_logger == nullptr) {
        g_logger = new Logger();
        
        // 保存初始配置
        g_logger->config_.log_dir = log_dir;
        g_logger->config_.log_file = log_file;
        g_logger->config_.async_mode = async_mode;
        g_logger->config_.level = level;
        g_logger->config_.verbose = verbose;
        g_logger->config_.allow_env_override = allow_env_override;

        // 如果允许环境变量覆盖，则加载环境变量
        if (allow_env_override) {
            g_logger->LoadFromEnv();
        }

        // 应用最终配置
        g_logger->SetAsyncMode(g_logger->config_.async_mode);
        g_logger->SetLevel(g_logger->config_.level);
        g_logger->SetVerbose(g_logger->config_.verbose);
        
        if (!g_logger->config_.log_file.empty()) {
            std::string full_path = g_logger->config_.log_dir + "/" + g_logger->config_.log_file;
            g_logger->SetFile(full_path);
        }
    }
}

void Logger::Shutdown() {
    if (g_logger) {
        delete g_logger;
        g_logger = nullptr;
    }
    spdlog::shutdown();
}

Logger& Logger::SetFile(const std::string& logfile, bool truncate) {
  file_sink_ = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logfile, truncate);
  UpdateLoggers();
  return *this;
}

Logger& Logger::SetAsyncMode(bool async_mode) {
  config_.async_mode = async_mode;
  UpdateLoggers();
  return *this;
}

Logger& Logger::SetLevel(spdlog::level::level_enum level) {
  config_.level = level;
  UpdateLoggers();
  return *this;
}

Logger& Logger::SetVerbose(Verbose ver) {
  switch (ver) {
  case Verbose::kLite: config_.pattern_ = "%v"; break;
  case Verbose::kLow: config_.pattern_ = "[%H:%M:%S.%f] %v"; break;
  case Verbose::kMedium: config_.pattern_ = "[%H:%M:%S.%f][%^%L%$][%@] %v"; break;
  case Verbose::kHigh: config_.pattern_ = "[%H:%M:%S.%f][%^%L%$][thread %t][%@] %v"; break;
  case Verbose::kFull: config_.pattern_ = "[%Y-%m-%d %H:%M:%S.%f][%^%L%$][thread %t][%!][%#] %v"; break;
  case Verbose::kUltra: config_.pattern_ = "[%Y-%m-%d %H:%M:%S.%F][%^%L%$][thread %t][%!][%@] %v"; break;
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
  } else if (config_.async_mode) {
    async_logger_->log(level, log_msg);
  } else {
    console_logger_->log(level, log_msg);
  }
  TriggerCallbacks(level, "", -1, "", log_msg);
}


void Logger::LoadFromEnv() {
    // 从环境变量读取配置
    const char* env_log_dir = std::getenv("SSLN_LOG_DIR");
    if (env_log_dir) {
        config_.log_dir = env_log_dir;
    }

    const char* env_log_file = std::getenv("SSLN_LOG_FILE");
    if (env_log_file) {
        config_.log_file = env_log_file;
    }

    const char* env_async = std::getenv("SSLN_LOG_ASYNC");
    if (env_async) {
        config_.async_mode = (std::string(env_async) == "1" || 
                            std::string(env_async) == "true");
    }

    const char* env_level = std::getenv("SSLN_LOG_LEVEL");
    if (env_level) {
        // 解析日志级别
        auto ParseLogLevel = [](const std::string& level) -> spdlog::level::level_enum {
          // Convert to uppercase for case-insensitive comparison
          std::string upper_level = level;
          std::transform(upper_level.begin(), upper_level.end(), upper_level.begin(), ::toupper);

          if (upper_level == "OFF" || upper_level == "6") return SSLOGGER_OFF;
          if (upper_level == "FATAL" || upper_level == "5") return SSLOGGER_FATAL;
          if (upper_level == "ERROR" || upper_level == "4") return SSLOGGER_ERROR;
          if (upper_level == "WARN" || upper_level == "3") return SSLOGGER_WARN;
          if (upper_level == "INFO" || upper_level == "2") return SSLOGGER_INFO;
          if (upper_level == "DEBUG" || upper_level == "1") return SSLOGGER_DEBUG;
          if (upper_level == "TRACE" || upper_level == "0") return SSLOGGER_TRACE;

          // If we get here, the input was invalid. Log a warning and return the default level.
          std::cerr << "Invalid log level: " << level << ". Using default level (INFO)." << std::endl;

          return spdlog::level::info;
        };

        config_.level = ParseLogLevel(env_level);
    }

    const char* env_verbose = std::getenv("SSLN_LOG_VERBOSE");
    if (env_verbose) {
        // 解析详细程度
        std::string verbose_str = env_verbose;
        if (verbose_str == "lite") config_.verbose = Verbose::kLite;
        else if (verbose_str == "low") config_.verbose = Verbose::kLow;
        else if (verbose_str == "medium") config_.verbose = Verbose::kMedium;
        else if (verbose_str == "high") config_.verbose = Verbose::kHigh;
        else if (verbose_str == "full") config_.verbose = Verbose::kFull;
        else if (verbose_str == "ultra") config_.verbose = Verbose::kUltra;
    }
}

template<typename LoggerType>
void Logger::UpdateLogger(std::shared_ptr<LoggerType>& logger) {
  if (logger) {
    logger->set_level(config_.level);
    logger->set_pattern(config_.pattern_);
  }
}

void Logger::UpdateLoggers() {
  if (file_sink_) {
    if (config_.async_mode) {
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
        spdlog::set_default_logger(sync_logger_);
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
  if (condition.level != SSLOGGER_OFF && condition.level != msg.level) {
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


} // namespace ssln

