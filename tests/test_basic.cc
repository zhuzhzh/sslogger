#include "vhlogger/vhlogger.h"


void callbackA(const vgp::LogContext& context) {
    std::cout << "Callback A triggered: " << context.message << std::endl;
}

void callbackB(const vgp::LogContext& context) {
    std::cout << "Callback B triggered: " << context.message << std::endl;
}

int main() {

    vgp::Logger& logger = vgp::Logger::GetInstance();

    // 为级别 4 的所有日志注册两个回调
    auto idA = logger.AddCallback(callbackA, 4);
    auto idB = logger.AddCallback(callbackB, 4);

    // 为特定消息注册两个回调
    logger.AddCallback(
        [](const vgp::LogContext& context) {
            std::cout << "Specific message callback 1" << std::endl;
        },
        vgp::INFO, "Important message"
    );
    logger.AddCallback(
        [](const vgp::LogContext& context) {
            std::cout << "Specific message callback 2" << std::endl;
        },
        vgp::INFO, "Important message"
    );

    // 记录一些日志
    int i = 999;
    vgp::log(4, "This is a debug message {}", i);
    vgp::log(4, "Important message");

    // 使用默认格式（只打印消息）
    logger.RemoveCallback(idA);
    vgp::log(4, "4 This is an info message");

    logger.ClearCallbacks(4);

    // 切换到带时间的格式
    vgp::Logger::GetInstance().SetFormat(vgp::Logger::Format::kMedium);
    std::vector<int> arr = {1,2,3,4};
    vgp::log(vgp::DEBUG, "5 This is a debug message with time: {}", arr);
    vgp::log(4, "4 This is a info message with time");
    vgp::log(2, "2 This is a warn message with time");

    // 切换到默认格式
    vgp::Logger::GetInstance().SetFormat(vgp::Logger::Format::kFull);
    vgp::log(1, "1 This is an error message with default format");

    // 将日志输出重定向到文件，并使用只有消息的格式
    //Logger::GetInstance().SetLogFile("app.log", true);
    vgp::Logger::GetInstance().SetFormat(vgp::Logger::Format::kLite);
    vgp::logf(2, "2 This is a warning message in file");
    vgp::Logger::GetInstance().SetFormat(vgp::Logger::Format::kMedium);
    vgp::logf(5, "5 This is a debug message in file with time");
    vgp::logf(4, "4 This is a info message in file with time");
    vgp::logf(2, "2 This is a warn message in file with time");
    vgp::logf(99, "99 This is a warn message in file with time");

   // print to console
    vgp::log(4, "4 test done");
    vgp::log(99, "This is a 99 warn message with time");

    VGP_CLOG(4, "this is one compile log");
    VGP_CLOGF(4, "this is one compile log in file");

    return 0;
}
