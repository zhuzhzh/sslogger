#ifndef VHLOGGER_HPP_H_
#define VHLOGGER_HPP_H_

#define SPDLOG_FMT_EXTERNAL
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>
#include <cstdlib>
#include <iostream>
#include <mutex>

class Logger {
public:
    enum class Format {
        LITE,
        MEDIUM,
        FULL
    };

    static Logger& GetInstance();

    void SetLogLevel(int verbosity);

    void SetLogFile(const std::string& filename, bool append = true);

    void SetFormat(Format format);

    template<typename... Args>
    void LogConsole(spdlog::level::level_enum level, spdlog::format_string_t<Args...> fmt, Args &&... args) {
        console_logger_->log(level, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void LogFile(spdlog::level::level_enum level, spdlog::format_string_t<Args...> fmt, Args &&... args) {
        if (file_logger_) {
            file_logger_->log(level, fmt, std::forward<Args>(args)...);
        } else {
            std::cerr << "Attempted to write to file logger before it was initialized." << std::endl;
        }
    }


private:
    Logger() : format_(Format::LITE) {
        const char* verbosity = std::getenv("UV_HYBRID_VERBOSITY");
        int level = verbosity ? std::atoi(verbosity) : 4;  // 默认为 info 级别
        
        console_logger_ = spdlog::stdout_color_mt("console");
        SetLogLevel(level);
        ApplyFormat(console_logger_);
    }

    void ApplyFormat(std::shared_ptr<spdlog::logger>& logger);

    static std::unique_ptr<Logger> instance_;
    std::shared_ptr<spdlog::logger> console_logger_;
    std::shared_ptr<spdlog::logger> file_logger_;
    Format format_;
};

// 使用宏来简化日志调用
#define LOG_TRACE(...) Logger::GetInstance().LogConsole(spdlog::level::trace, __VA_ARGS__)
#define LOG_DEBUG(...) Logger::GetInstance().LogConsole(spdlog::level::debug, __VA_ARGS__)
#define LOG_INFO(...)  Logger::GetInstance().LogConsole(spdlog::level::info, __VA_ARGS__)
#define LOG_WARN(...)  Logger::GetInstance().LogConsole(spdlog::level::warn, __VA_ARGS__)
#define LOG_ERROR(...) Logger::GetInstance().LogConsole(spdlog::level::err, __VA_ARGS__)
#define LOG_CRITICAL(...) Logger::GetInstance().LogConsole(spdlog::level::critical, __VA_ARGS__)

#define LOGF_TRACE(...) Logger::GetInstance().LogFile(spdlog::level::trace, __VA_ARGS__)
#define LOGF_DEBUG(...) Logger::GetInstance().LogFile(spdlog::level::debug, __VA_ARGS__)
#define LOGF_INFO(...)  Logger::GetInstance().LogFile(spdlog::level::info, __VA_ARGS__)
#define LOGF_WARN(...)  Logger::GetInstance().LogFile(spdlog::level::warn, __VA_ARGS__)
#define LOGF_ERROR(...) Logger::GetInstance().LogFile(spdlog::level::err, __VA_ARGS__)
#define LOGF_CRITICAL(...) Logger::GetInstance().LogFile(spdlog::level::critical, __VA_ARGS__)

#endif /* end of include guard: VHLOGGER_HPP_H_ */
