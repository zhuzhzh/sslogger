#include "vhlogger/vhlogger.hpp"
#include <mutex>


Logger& Logger::GetInstance() {
    static std::once_flag flag;
    std::call_once(flag, []() { instance_.reset(new Logger()); });
    return *instance_;
}


void Logger::SetLogLevel(int verbosity) {
    spdlog::level::level_enum level;
    if (verbosity <= 0) level = spdlog::level::off;
    else if (verbosity >= 6) level = spdlog::level::trace;
    else {
        switch(verbosity) {
            case 1: level = spdlog::level::critical; break;
            case 2: level = spdlog::level::err; break;
            case 3: level = spdlog::level::warn; break;
            case 4: level = spdlog::level::info; break;
            case 5: level = spdlog::level::debug; break;
            default: level = spdlog::level::info; // 默认情况
        }
    }
    
    console_logger_->set_level(level);
    if (file_logger_) file_logger_->set_level(level);
}

void Logger::SetLogFile(const std::string& filename, bool append) {
    try {
        file_logger_ = spdlog::basic_logger_mt("file_logger", filename, !append);
        ApplyFormat(file_logger_);
        file_logger_->set_level(console_logger_->level()); // 设置与控制台相同的日志级别
    } catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "Log file initialization failed: " << ex.what() << std::endl;
    }
}

void Logger::SetFormat(Format format) {
    format_ = format;
    ApplyFormat(console_logger_);
    if (file_logger_) ApplyFormat(file_logger_);
}

void Logger::ApplyFormat(std::shared_ptr<spdlog::logger>& logger) {
    switch (format_) {
        case Format::LITE:
            logger->set_pattern("%v");
            break;
        case Format::MEDIUM:
            logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] %v");
            break;
        case Format::FULL:
            logger->set_pattern("%+");
            break;
    }
}

std::unique_ptr<Logger> Logger::instance_;
