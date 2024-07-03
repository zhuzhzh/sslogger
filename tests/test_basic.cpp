#include "vhlogger/vhlogger.hpp"

int main() {
    // 使用默认格式（只打印消息）
    LOG_INFO("This is an info message");

    // 切换到带时间的格式
    Logger::GetInstance().SetFormat(Logger::Format::MEDIUM);
    LOG_TRACE("This is a trace message with time");
    LOG_DEBUG("This is a debug message with time");
    LOG_INFO("This is a info message with time");
    LOG_WARN("This is a warn message with time");
    LOG_CRITICAL("This is a critical message with time");

    // 切换到spdlog默认格式
    Logger::GetInstance().SetFormat(Logger::Format::FULL);
    LOG_ERROR("This is an error message with spdlog default format");

    // 将日志输出重定向到文件，并使用只有消息的格式
    Logger::GetInstance().SetLogFile("app.log", true);
    Logger::GetInstance().SetFormat(Logger::Format::LITE);
    LOGF_WARN("This is a warning message in file");
    Logger::GetInstance().SetFormat(Logger::Format::MEDIUM);
    LOGF_TRACE("This is a trace message with time");
    LOGF_DEBUG("This is a debug message with time");
    LOGF_INFO("This is a info message with time");
    LOGF_WARN("This is a warn message with time");
    LOGF_CRITICAL("This is a critical message with time");

    return 0;
}
