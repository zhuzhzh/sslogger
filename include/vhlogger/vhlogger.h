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
#include <spdlog/fmt/bin_to_hex.h>
#include <memory>
#include <vector>
#include <functional>
#include <string>

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

    Logger& SetLevel(spdlog::level::level_enum level) {
      level_ = level;
      console_logger_->set_level(level);
      if (file_logger_) file_logger_->set_level(level);
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
      console_logger_->set_pattern(pattern_);
      if (file_logger_) file_logger_->set_pattern(pattern_);
      return *this;
    }

    void AddCallback(const CallbackCondition& condition, CallbackFunction callback) {
      callbacks_.push_back({condition, callback});
    }

    template<typename... Args>
    void Log(spdlog::level::level_enum level, const char* file, int line, const char* func, const char* fmt, const Args&... args) {
        console_logger_->log(spdlog::source_loc{file, line, func}, level, fmt, args...);
#ifdef VHLOGGER_ENABLE_CB
        TriggerCallbacks(level, "", -1, "", fmt::format(fmt, args...));
#endif
    }

    template<typename... Args>
    void LogF(spdlog::level::level_enum level, const char* file, int line, const char* func, const char* fmt, const Args&... args) {
        if (file_logger_) {
            file_logger_->log(spdlog::source_loc{file, line, func}, level, fmt, args...);
        }
#ifdef VHLOGGER_ENABLE_CB
        TriggerCallbacks(level, "", -1, "", fmt::format(fmt, args...));
#endif
    }

    template<typename... Args>
      void Debug(const char* fmt, const Args&... args) {
        Log(VHLOGGER_DEBUG, fmt, args...);
      }

    template<typename... Args>
      void DebugF(const char* fmt, const Args&... args) {
        LogF(VHLOGGER_DEBUG, fmt, args...);
      }

    template<typename... Args>
      void Trace(const char* fmt, const Args&... args) {
        Log(VHLOGGER_TRACE, fmt, args...);
      }

    template<typename... Args>
      void TraceF(const char* fmt, const Args&... args) {
        LogF(VHLOGGER_TRACE, fmt, args...);
      }

    template<typename... Args>
      void Info(const char* fmt, const Args&... args) {
        Log(VHLOGGER_INFO, fmt, args...);
      }

    template<typename... Args>
      void InfoF(const char* fmt, const Args&... args) {
        LogF(VHLOGGER_INFO, fmt, args...);
      }
    
    template<typename... Args>
      void Warn(const char* fmt, const Args&... args) {
        Log(VHLOGGER_WARN, fmt, args...);
      }

    template<typename... Args>
      void WarnF(const char* fmt, const Args&... args) {
        LogF(VHLOGGER_WARN, fmt, args...);
      }

    template<typename... Args>
      void Error(const char* fmt, const Args&... args) {
        Log(VHLOGGER_ERROR, fmt, args...);
      }

    template<typename... Args>
      void ErrorF(const char* fmt, const Args&... args) {
        LogF(VHLOGGER_ERROR, fmt, args...);
      }

    template<typename... Args>
      void Fatal(const char* fmt, const Args&... args) {
        Log(VHLOGGER_FATAL, fmt, args...);
      }

    template<typename... Args>
      void FatalF(const char* fmt, const Args&... args) {
        Log(VHLOGGER_FATAL, fmt, args...);
      }

    void LogArray(spdlog::level::level_enum level, const uint8_t* ptr, int size, bool to_file = false) {
      auto log_msg = fmt::format("Array data: {}", spdlog::to_hex(ptr, ptr + size));
      if (to_file && file_logger_) {
        file_logger_->log(level, log_msg);
      } else {
        console_logger_->log(level, log_msg);
      }
      TriggerCallbacks(level, "", -1, "", log_msg);
    }

  private:
    Logger() {
      console_sink_ = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
      console_logger_ = std::make_shared<spdlog::logger>("console", console_sink_);
      spdlog::set_default_logger(console_logger_);
      SetupFromEnv();
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

    void UpdateLoggers() {
      if (file_sink_) {
        file_logger_ = std::make_shared<spdlog::logger>("file_logger", file_sink_);
        file_logger_->set_level(level_);
        file_logger_->set_pattern(pattern_);
      }
    }

    template<typename... Args>
      void Log(spdlog::level::level_enum level, const char* fmt, const Args&... args) {
        console_logger_->log(level, fmt, args...);
#ifdef VHLOGGER_ENABLE_CB
        TriggerCallbacks(level, "", -1, "", fmt::format(fmt, args...));
#endif
      }

    template<typename... Args>
      void LogF(spdlog::level::level_enum level, const char* fmt, const Args&... args) {
        if (file_logger_) {
          file_logger_->log(level, fmt, args...);
#ifdef VHLOGGER_ENABLE_CB
          TriggerCallbacks(level, "", -1, "", fmt::format(fmt, args...));
#endif
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
    std::shared_ptr<spdlog::logger> file_logger_;
    std::string pattern_;
    Verbose verbose_;
    spdlog::level::level_enum level_;
    std::string filename_;
    std::vector<std::pair<CallbackCondition, CallbackFunction>> callbacks_;

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
#define VGP_LOG_F(level, ...) vgp::Logger::GetInstance()->LogF(level, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define VGP_TRACEF(...) VGP_LOG_F(VHLOGGER_TRACE, __VA_ARGS__)
#define VGP_DEBUGF(...) VGP_LOG_F(VHLOGGER_DEBUG, __VA_ARGS__)
#define VGP_INFOF(...) VGP_LOG_F(VHLOGGER_INFO, __VA_ARGS__)
#define VGP_WARNF(...) VGP_LOG_F(VHLOGGER_WARN, __VA_ARGS__)
#define VGP_ERRORF(...) VGP_LOG_F(VHLOGGER_ERROR, __VA_ARGS__)
#define VGP_FATALF(...) VGP_LOG_F(VHLOGGER_FATAL, __VA_ARGS__)

#endif  // VGP_VHLOGGER_H_
