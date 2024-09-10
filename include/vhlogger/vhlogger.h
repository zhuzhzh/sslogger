// vhlogger.h
#ifndef VGP_VHLOGGER_H_
#define VGP_VHLOGGER_H_

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

#define VHLOGGER_TRACE    spdlog::level::trace
#define VHLOGGER_DEBUG    spdlog::level::debug
#define VHLOGGER_INFO     spdlog::level::info
#define VHLOGGER_WARN     spdlog::level::warn
#define VHLOGGER_ERROR    spdlog::level::err
#define VHLOGGER_FATAL    spdlog::level::critical
#define VHLOGGER_OFF      spdlog::level::off

namespace vgp {

  class Logger {
  public:
    enum class Verbose { kLite, kLow, kMedium, kHigh, kFull, kUltra};

    using CallbackFunction = std::function<void(const spdlog::details::log_msg&)>;

    struct CallbackCondition {
      spdlog::level::level_enum level = VHLOGGER_OFF;
      tl::optional<std::string> file;  // 使用tl::optional表示可选项
      tl::optional<int> line;
      tl::optional<std::string> function;
      tl::optional<std::string> message;
    };

    static Logger* GetInstance() {
      static Logger instance;
      return &instance;
    }

    Logger& SetFile(const std::string& filename, bool truncate = true) {
      filename_ = filename;
      file_sink_ = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename,truncate);
      UpdateLoggers();
      return *this;
    }

    Logger& SetAsyncMode(bool async_mode) {
      async_mode_ = async_mode;
      UpdateLoggers();
      return *this;
    }


    Logger& SetLevel(spdlog::level::level_enum level) {
      level_ = level;
      UpdateLoggers();
      return *this;
    }

    Logger& SetVerbose(Verbose ver) {
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

    Logger& AddLogger(const std::string& name, const std::string& filename, bool daily = false) {
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

    void AddCallback(const CallbackCondition& condition, CallbackFunction callback) {
      callbacks_.push_back({condition, callback});
    }

    template<typename... Args>
    void Log(spdlog::level::level_enum level, const char* file, int line, const char* func, const char* fmt, const Args&... args) {
      if (async_mode_) {
        async_logger_->log(spdlog::source_loc{file, line, func}, level, fmt, args...);
      } else {
        console_logger_->log(spdlog::source_loc{file, line, func}, level, fmt, args...);
      }
#ifdef VHLOGGER_ENABLE_CB
        TriggerCallbacks(level, "", -1, "", fmt::format(fmt, args...));
#endif
    }

    template<typename... Args>
    void LogF(spdlog::level::level_enum level, const char* file, int line, const char* func, const char* fmt, const Args&... args) {
      if (async_mode_) {
        async_logger_->log(spdlog::source_loc{file, line, func}, level, fmt, args...);
      } else {
        sync_logger_->log(spdlog::source_loc{file, line, func}, level, fmt, args...);
      }
#ifdef VHLOGGER_ENABLE_CB
        TriggerCallbacks(level, "", -1, "", fmt::format(fmt, args...));
#endif
    }

    template<typename... Args>
    void LogTo(const std::string& logger_name, spdlog::level::level_enum level, const char* file, int line, const char* func, const char* fmt, const Args&... args) {
      auto it = loggers_.find(logger_name);
      if (it != loggers_.end()) {
        it->second->log(spdlog::source_loc{file, line, func}, level, fmt, args...);
      }
#ifdef VHLOGGER_ENABLE_CB
        TriggerCallbacks(level, "", -1, "", fmt::format(fmt, args...));
#endif
    }

    void LogArray(spdlog::level::level_enum level, const uint8_t* ptr, int size, bool to_file = false) {
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

  private:
    Logger():async_mode_(false) {
      console_sink_ = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
      console_logger_ = std::make_shared<spdlog::logger>("console", console_sink_);
      spdlog::set_default_logger(console_logger_);
      SetupFromEnv();
      UpdateLoggers();
    }

    void SetupFromEnv() {
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

    spdlog::level::level_enum ParseLogLevel(const std::string& level) {
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
      void UpdateLogger(std::shared_ptr<LoggerType>& logger) {
        if (logger) {
          logger->set_level(level_);
          logger->set_pattern(pattern_);
        }
      }

    void UpdateLoggers() {
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

    void TriggerCallbacks(spdlog::level::level_enum level, const std::string& file, int line,
      const std::string& function, const std::string& message) {
      spdlog::details::log_msg msg;
      msg.level = level;
      msg.time = std::chrono::system_clock::now();
      msg.source.filename = file.c_str();
      msg.source.line = line;
      msg.source.funcname = function.c_str();
      msg.payload = message;

      for (const auto& cb : callbacks_) {
        if (MatchesCondition(cb.first, msg)) {
          cb.second(msg);
        }
      }
    }

    bool MatchesCondition(const CallbackCondition& condition, const spdlog::details::log_msg& msg) {
      // 如果 level 被设置，则必须匹配
      if (condition.level != VHLOGGER_OFF && condition.level != msg.level) {
        return false;
      }
      // 如果 file 被设置，则必须匹配
      if (condition.file.has_value() && msg.source.filename != condition.file.value()) {
        return false;
      }
      // 如果 line 被设置，则必须匹配
      if (condition.line.has_value() && msg.source.line != condition.line.value()) {
        return false;
      }
      // 如果 function 被设置，则必须匹配
      if (condition.function.has_value() && msg.source.funcname != condition.function.value()) {
        return false;
      }
      // 如果 message 被设置，则必须包含在日志消息中
      if (condition.message.has_value() && std::string(msg.payload.data(), msg.payload.size()).find(condition.message.value()) == std::string::npos) {
        return false;
      }
      return true;
    }

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
#define VGP_LOG_ARRAY(level, ptr, size) vgp::Logger::GetInstance()->LogArray(level, ptr, size, false)
#define VGP_LOG_ARRAY_F(level, ptr, size) vgp::Logger::GetInstance()->LogArray(level, ptr, size, true)

#define VGP_LOG(level, ...) vgp::Logger::GetInstance()->Log(level, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define VGP_TRACE(...) VGP_LOG(VHLOGGER_TRACE, __VA_ARGS__)
#define VGP_DEBUG(...) VGP_LOG(VHLOGGER_DEBUG, __VA_ARGS__)
#define VGP_INFO(...) VGP_LOG(VHLOGGER_INFO, __VA_ARGS__)
#define VGP_WARN(...) VGP_LOG(VHLOGGER_WARN, __VA_ARGS__)
#define VGP_ERROR(...) VGP_LOG(VHLOGGER_ERROR, __VA_ARGS__)
#define VGP_FATAL(...) VGP_LOG(VHLOGGER_FATAL, __VA_ARGS__)

// File logging macros (if needed)
#define VGP_LOG_TO(logger_name, level, ...) vgp::Logger::GetInstance()->LogTo(logger_name, level, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define VGP_TRACE_TO(logger_name, ...) VGP_LOG__TO(logger_name, VHLOGGER_TRACE, __VA_ARGS__)
#define VGP_DEBUG_TO(logger_name, ...) VGP_LOG__TO(logger_name, VHLOGGER_DEBUG, __VA_ARGS__)
#define VGP_INFO_TO(logger_name, ...) VGP_LOG__TO(logger_name, VHLOGGER_INFO, __VA_ARGS__)
#define VGP_WARN_TO(logger_name, ...) VGP_LOG__TO(logger_name, VHLOGGER_WARN, __VA_ARGS__)
#define VGP_ERROR_TO(logger_name, ...) VGP_LOG__TO(logger_name, VHLOGGER_ERROR, __VA_ARGS__)
#define VGP_FATAL_TO(logger_name, ...) VGP_LOG__TO(logger_name, VHLOGGER_FATAL, __VA_ARGS__)

#define VGP_LOG_F(level, ...) vgp::Logger::GetInstance()->LogF(level, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define VGP_TRACEF(...) VGP_LOG_F(VHLOGGER_TRACE, __VA_ARGS__)
#define VGP_DEBUGF(...) VGP_LOG_F(VHLOGGER_DEBUG, __VA_ARGS__)
#define VGP_INFOF(...) VGP_LOG_F(VHLOGGER_INFO, __VA_ARGS__)
#define VGP_WARNF(...) VGP_LOG_F(VHLOGGER_WARN, __VA_ARGS__)
#define VGP_ERRORF(...) VGP_LOG_F(VHLOGGER_ERROR, __VA_ARGS__)
#define VGP_FATALF(...) VGP_LOG_F(VHLOGGER_FATAL, __VA_ARGS__)

#endif  // VGP_VHLOGGER_H_
