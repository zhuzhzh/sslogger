// vhlogger.h
#ifndef VGP_VHLOGGER_H_
#define VGP_VHLOGGER_H_

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/fmt/bin_to_hex.h>
#include <memory>
#include <vector>
#include <functional>
#include <string>

#define LOGLEVEL_TRACE    spdlog::level::trace
#define LOGLEVEL_DEBUG    spdlog::level::debug
#define LOGLEVEL_INFO     spdlog::level::info
#define LOGLEVEL_WARN     spdlog::level::warn
#define LOGLEVEL_ERROR    spdlog::level::err
#define LOGLEVEL_FATAL spdlog::level::critical
#define LOGLEVEL_OFF      spdlog::level::off

namespace vgp {

  class Logger {
  public:
    enum class Format { kLite, kMedium, kFull };

    using CallbackFunction = std::function<void(const spdlog::details::log_msg&)>;

    struct CallbackCondition {
      spdlog::level::level_enum level = LOGLEVEL_OFF;
      std::string file;
      int line = -1;
      std::string function;
      std::string message;
    };

    static Logger* GetInstance() {
      static Logger instance;
      return &instance;
    }

    Logger& SetLogFile(const std::string& filename, bool append = false) {
      file_sink_ = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, append);
      UpdateLoggers();
      return *this;
    }

    Logger& SetLogLevel(spdlog::level::level_enum level) {
      console_logger_->set_level(level);
      if (file_logger_) file_logger_->set_level(level);
      return *this;
    }

    Logger& SetFormat(Format format) {
      switch (format) {
      case Format::kLite: pattern_ = "%v"; break;
      case Format::kMedium: pattern_ = "[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v"; break;
      case Format::kFull: pattern_ = "[%Y-%m-%d %H:%M:%S.%e %z] [%n] [%^---%L---%$] [thread %t] [%!] %v"; break;
      }
      console_logger_->set_pattern(pattern_);
      if (file_logger_) file_logger_->set_pattern(pattern_);
      return *this;
    }

    void AddCallback(const CallbackCondition& condition, CallbackFunction callback) {
      callbacks_.push_back({condition, callback});
    }

    template<typename... Args>
      void Debug(const char* fmt, const Args&... args) {
        Log(LOGLEVEL_DEBUG, fmt, args...);
      }

    template<typename... Args>
      void DebugF(const char* fmt, const Args&... args) {
        LogF(LOGLEVEL_DEBUG, fmt, args...);
      }

    template<typename... Args>
      void Trace(const char* fmt, const Args&... args) {
        Log(LOGLEVEL_TRACE, fmt, args...);
      }

    template<typename... Args>
      void TraceF(const char* fmt, const Args&... args) {
        LogF(LOGLEVEL_TRACE, fmt, args...);
      }

    template<typename... Args>
      void Info(const char* fmt, const Args&... args) {
        Log(LOGLEVEL_INFO, fmt, args...);
      }

    template<typename... Args>
      void InfoF(const char* fmt, const Args&... args) {
        LogF(LOGLEVEL_INFO, fmt, args...);
      }

    template<typename... Args>
      void Error(const char* fmt, const Args&... args) {
        Log(LOGLEVEL_ERROR, fmt, args...);
      }

    template<typename... Args>
      void ErrorF(const char* fmt, const Args&... args) {
        LogF(LOGLEVEL_ERROR, fmt, args...);
      }

    template<typename... Args>
      void Fatal(const char* fmt, const Args&... args) {
        Log(LOGLEVEL_FATAL, fmt, args...);
      }

    template<typename... Args>
      void FatalF(const char* fmt, const Args&... args) {
        Log(LOGLEVEL_FATAL, fmt, args...);
      }

    // Implement Trace, Info, Warn, Error, Fatal similarly...

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
    }

    void UpdateLoggers() {
      if (file_sink_) {
        file_logger_ = std::make_shared<spdlog::logger>("file_logger", file_sink_);
        file_logger_->set_level(console_logger_->level());
        file_logger_->set_pattern(pattern_);
      }
    }


    template<typename... Args>
      void Log(spdlog::level::level_enum level, const char* fmt, const Args&... args) {
        console_logger_->log(level, fmt, args...);
        TriggerCallbacks(level, "", -1, "", fmt::format(fmt, args...));
      }

    template<typename... Args>
      void LogF(spdlog::level::level_enum level, const char* fmt, const Args&... args) {
        if (file_logger_) {
          file_logger_->log(level, fmt, args...);
          TriggerCallbacks(level, "", -1, "", fmt::format(fmt, args...));
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

    bool MatchesCondition(const CallbackCondition& condition,
      const spdlog::details::log_msg& msg) {
      if (condition.level != LOGLEVEL_OFF && condition.level != msg.level) {
        return false;
      }
      if (!condition.file.empty() && msg.source.filename != condition.file) {
        return false;
      }
      if (condition.line != -1 && msg.source.line != condition.line) {
        return false;
      }
      if (!condition.function.empty() && msg.source.funcname != condition.function) {
        return false;
      }
      if (!condition.message.empty() && msg.payload.compare(condition.message) == std::string::npos) {
        return false;
      }
      return true;
    }

    std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> console_sink_;
    std::shared_ptr<spdlog::sinks::basic_file_sink_mt> file_sink_;
    std::shared_ptr<spdlog::logger> console_logger_;
    std::shared_ptr<spdlog::logger> file_logger_;
    std::string pattern_;
    std::vector<std::pair<CallbackCondition, CallbackFunction>> callbacks_;

  };

} // namespace vgp

// Macro definitions
#define VGP_DEBUG(...) vgp::Logger::GetInstance()->Debug(__VA_ARGS__)
#define VGP_TRACE(...) vgp::Logger::GetInstance()->Trace(__VA_ARGS__)
#define VGP_INFO(...) vgp::Logger::GetInstance()->Info(__VA_ARGS__)
#define VGP_WARN(...) vgp::Logger::GetInstance()->Warn(__VA_ARGS__)
#define VGP_ERROR(...) vgp::Logger::GetInstance()->Error(__VA_ARGS__)
#define VGP_FATAL(...) vgp::Logger::GetInstance()->Fatal(__VA_ARGS__)

#define VGP_DEBUGF(...) vgp::Logger::GetInstance()->DebugF(__VA_ARGS__)
#define VGP_TRACEF(...) vgp::Logger::GetInstance()->TraceF(__VA_ARGS__)
#define VGP_INFOF(...) vgp::Logger::GetInstance()->InfoF(__VA_ARGS__)
#define VGP_WARNF(...) vgp::Logger::GetInstance()->WarnF(__VA_ARGS__)
#define VGP_ERRORF(...) vgp::Logger::GetInstance()->ErrorF(__VA_ARGS__)
#define VGP_FATALF(...) vgp::Logger::GetInstance()->FatalF(__VA_ARGS__)

#define VGP_LOG_ARRAY(level, ptr, size) vgp::Logger::GetInstance()->LogArray(vgp::Logger::Level::level, ptr, size, false)
#define VGP_LOG_ARRAY_F(level, ptr, size) vgp::Logger::GetInstance()->LogArray(vgp::Logger::Level::level, ptr, size, true)

#endif  // VGP_VHLOGGER_H_
