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
    vgp::log(4, "This is a debug message {}\n", i);
    vgp::log(4, "Important message\n");

    // 使用默认格式（只打印消息）
    logger.RemoveCallback(idA);
    vgp::log(4, "4 This is an info message\n");

    logger.ClearCallbacks(4);

    // 切换到带时间的格式
    vgp::Logger::GetInstance().SetFormat(vgp::Logger::Format::kMedium);
    std::vector<int> arr = {1,2,3,4};
    vgp::logn(vgp::DEBUG, "5 This is a debug message with time: {}", arr);
    vgp::log(4, "4 This is a info message with time\n");
    vgp::logn(2, "2 This is a warn message with time");

    // 切换到默认格式
    vgp::Logger::GetInstance().SetFormat(vgp::Logger::Format::kFull);
    vgp::logn(1, "1 This is an error message with default format");

    // 将日志输出重定向到文件，并使用只有消息的格式
    //Logger::GetInstance().SetLogFile("app.log", true);
    vgp::Logger::GetInstance().SetFormat(vgp::Logger::Format::kLite);
    vgp::logfn(2, "2 This is a warning message in file");
    vgp::Logger::GetInstance().SetFormat(vgp::Logger::Format::kMedium);
    vgp::logfn(5, "5 This is a debug message in file with time");
    vgp::logf(4, "4 This is a info message in file with time\n");
    vgp::logfn(2, "2 This is a warn message in file with time");
    vgp::logf(99, "99 This is a warn message in file with time\n");

   // print to console
    vgp::log(4, "4 test done\n");
    vgp::log(99, "This is a 99 warn message with time\n");

    VGP_CLOG(4, "this is one compile log\n");
    VGP_CLOGFN(4, "this is one compile log in file");

    uint8_t data[] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 
                  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                  0x99, 0xAA, 0xBB, 0xCC};
    size_t size = sizeof(data);

    vgp::logf_array(vgp::INFO, data, size);

    return 0;
}
