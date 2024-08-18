#include "vhlogger/vhlogger.h"

using Level = vgp::Logger::Level;

void callbackA(const vgp::LogContext& context) {
    std::cout << "Callback A triggered: " << context.message << std::endl;
}

void callbackB(const vgp::LogContext& context) {
    std::cout << "Callback B triggered: " << context.message << std::endl;
}

int main() {

    vgp::Logger& logger = vgp::Logger::GetInstance();

    // 为级别 4 的所有日志注册两个回调
    auto idA = logger.AddCallback(callbackA, Level::INFO);
    auto idB = logger.AddCallback(callbackB, Level::INFO);

    // 为特定消息注册两个回调
    logger.AddCallback(
        [](const vgp::LogContext& context) {
            std::cout << "Specific message callback 1" << std::endl;
        },
        Level::INFO, "Important message"
    );
    logger.AddCallback(
        [](const vgp::LogContext& context) {
            std::cout << "Specific message callback 2" << std::endl;
        },
        Level::INFO, "Important message"
    );

    // 记录一些日志
    int i = 999;
    VGP_LOG(Level::INFO, "This is a debug message {}", i);
    VGP_LOG(Level::INFO, "Important message");

    // 使用默认格式（只打印消息）
    logger.RemoveCallback(idA);
    VGP_INFO("4 This is an info message");
    vgp::logger().Info(VGP_LOG_LOC, false, "4 This is an info message");


    logger.ClearCallbacks(Level::INFO);

    // 切换到带时间的格式
    vgp::Logger::GetInstance().SetFormat(vgp::Logger::Format::kMedium);
    std::vector<int> arr = {1,2,3,4};
    VGP_LOG(Level::DEBUG, "5 This is a debug message with time: {}", arr);
    VGP_INFO("4 This is a info message with time");
    VGP_LOG(Level::ERROR, "2 This is a error message with time");

    // 切换到默认格式
    vgp::Logger::GetInstance().SetFormat(vgp::Logger::Format::kFull);
    VGP_LOG(Level::FATAL, "1 This is an fatal message with default format");
    VGP_INFO("56, This is an info message with full format");

    // 将日志输出重定向到文件，并使用只有消息的格式
    //Logger::GetInstance().SetLogFile("app.log", true);
    vgp::Logger::GetInstance().SetFormat(vgp::Logger::Format::kLite);
    VGP_LOGF(Level::ERROR, "2 This is a error message in file");
    vgp::Logger::GetInstance().SetFormat(vgp::Logger::Format::kMedium);
    VGP_LOGF(Level::DEBUG, "5 This is a debug message in file with time");
    VGP_LOGF(Level::INFO, "4 This is a info message in file with time");
    VGP_LOGF(Level::WARN, "2 This is a warn message in file with time");
    VGP_LOGF(Level::TRACE, "99 This is a trace message in file with time");

   // print to console
    VGP_INFO("4 test done");
    VGP_WARN("This is a 99 warn message with time");

    VGP_CLOG(Level::INFO, "this is an info message in one compile log");
    VGP_CLOGF(Level::INFO, "this is an info message in one compile log in file");

    uint8_t data[] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 
                  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                  0x99, 0xAA, 0xBB, 0xCC};
    size_t size = sizeof(data);

    VGP_LOGF_ARRAY(Level::INFO, data, size);

    uint8_t data1[] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 
                  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                  0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xFF, 0x12, 0x34,
                  0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x01, 0x23,
                  0x34, 0x45, 0x56 };
    size_t size1 = sizeof(data1);

    vgp::Logger::GetInstance().SetFormat(vgp::Logger::Format::kFull);
    VGP_LOG_ARRAY(Level::INFO, data1, size1);

    VGP_WARNF("This is a warn message with time in file or console");
    vgp::Logger::GetInstance().SetFormat(vgp::Logger::Format::kLite);
    VGP_DEBUGF("1 This is a debug message with time in file or console with lite format");
    vgp::Logger::GetInstance().SetFormat(vgp::Logger::Format::kMedium);
    VGP_DEBUGF("2 This is a debug message with time in file or console with lite format");
    vgp::Logger::GetInstance().SetFormat(vgp::Logger::Format::kFull);
    VGP_DEBUGF("3 This is a debug message with time in file or console with lite format");

    return 0;
}
