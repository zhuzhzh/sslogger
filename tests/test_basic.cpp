#include "vhlogger/vhlogger.hpp"

using namespace vgp;

int main() {
    // 使用默认格式（只打印消息）
    LOG(4, "4 This is an info message");

    // 切换到带时间的格式
    Logger::GetInstance().SetFormat(Logger::Format::kMedium);
    LOG(5, "5 This is a debug message with time");
    LOG(4, "4 This is a info message with time");
    LOG(2, "2 This is a warn message with time");

    // 切换到默认格式
    Logger::GetInstance().SetFormat(Logger::Format::kFull);
    LOG(1, "1 This is an error message with default format");

    // 将日志输出重定向到文件，并使用只有消息的格式
    Logger::GetInstance().SetLogFile("app.log", true);
    Logger::GetInstance().SetFormat(Logger::Format::kLite);
    LOGF(2, "2 This is a warning message in file");
    Logger::GetInstance().SetFormat(Logger::Format::kMedium);
    LOGF(5, "5 This is a debug message in file with time");
    LOGF(4, "4 This is a info message in file with time");
    LOGF(2, "2 This is a warn message in file with time");
    LOGF(99, "99 This is a warn message in file with time");

   // print to console
    LOG(4, "4 test done");
    LOG(99, "This is a 99 warn message with time");

    return 0;
}
