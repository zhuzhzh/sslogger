#include "vhlogger/vhlogger.hpp"

using namespace vgp;

int main() {
    // 使用默认格式（只打印消息）
    LOG_INFO("This is an info message");

    // 切换到带时间的格式
    Logger::GetInstance().SetFormat(Logger::Format::MEDIUM);
    LOG_DEBUG("This is a debug message with time");
    LOG_INFO("This is a info message with time");
    LOG_WARN("This is a warn message with time");

    // 切换到默认格式
    Logger::GetInstance().SetFormat(Logger::Format::FULL);
    LOG_ERROR("This is an error message with default format");

    // 将日志输出重定向到文件，并使用只有消息的格式
    Logger::GetInstance().SetLogFile("app.log", true);
    Logger::GetInstance().SetFormat(Logger::Format::LITE);
    LOGF_WARN("This is a warning message in file");
    Logger::GetInstance().SetFormat(Logger::Format::MEDIUM);
    LOGF_DEBUG("This is a debug message in filewith time");
    LOGF_INFO("This is a info message in file with time");
    LOGF_WARN("This is a warn message in file with time");


    LOG_INFO("test done");

    return 0;
}
