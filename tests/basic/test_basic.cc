#include "ssln/sslogger.h"

int main() {

    ssln::Logger* logger = ssln::Logger::GetInstance();
    logger->SetFile("basic.log");


    // 记录一些日志
    int i = 999;
    SSLN_INFO("This is a debug message {}", i);
    SSLN_INFO("Important message");

    // 使用默认格式（只打印消息）
    SSLN_INFO("4 This is an info message");
    SSLN_INFO("4 This is an info message");


    // 切换到带时间的格式
    ssln::Logger::GetInstance()->SetVerbose(ssln::Logger::Verbose::kMedium);
    std::vector<int> arr = {1,2,3,4};
    SSLN_DEBUG("5 This is a debug message with time: {}", arr);
    SSLN_INFO("4 This is a info message with time");
    SSLN_ERROR("2 This is a error message with time");

    // 切换到默认格式
    ssln::Logger::GetInstance()->SetVerbose(ssln::Logger::Verbose::kFull);
    SSLN_FATAL("1 This is an fatal message with default format");
    SSLN_INFO("56, This is an info message with full format");

    // 将日志输出重定向到文件，并使用只有消息的格式
    //Logger::GetInstance().SetLogFile("app.log", true);
    ssln::Logger::GetInstance()->SetVerbose(ssln::Logger::Verbose::kLite);
    SSLN_ERRORF("2 This is a error message in file");
    ssln::Logger::GetInstance()->SetVerbose(ssln::Logger::Verbose::kMedium);
    SSLN_DEBUGF("5 This is a debug message in file with time");
    SSLN_INFOF("4 This is a info message in file with time");
    SSLN_WARNF("2 This is a warn message in file with time");
    SSLN_TRACEF("99 This is a trace message in file with time");

   // print to console
    SSLN_INFO("4 test done");
    SSLN_WARN("This is a 99 warn message with time");

    //SSLN_CLOG(Level::INFO, "this is an info message in one compile log");
    //SSLN_CLOGF(Level::INFO, "this is an info message in one compile log in file");

    uint8_t data[] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 
                  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                  0x99, 0xAA, 0xBB, 0xCC};
    size_t size = sizeof(data);

    SSLN_LOGF_ARRAY(SSLOGGER_INFO, data, size);

    uint8_t data1[] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 
                  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                  0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xFF, 0x12, 0x34,
                  0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x01, 0x23,
                  0x34, 0x45, 0x56 };
    size_t size1 = sizeof(data1);

    ssln::Logger::GetInstance()->SetVerbose(ssln::Logger::Verbose::kFull);
    SSLN_LOG_ARRAY(SSLOGGER_DEBUG, data1, size1);

    SSLN_WARNF("This is a warn message with time in file or console");
    ssln::Logger::GetInstance()->SetVerbose(ssln::Logger::Verbose::kLite);
    SSLN_DEBUGF("1 This is a debug message with time in file or console with lite format");
    ssln::Logger::GetInstance()->SetVerbose(ssln::Logger::Verbose::kMedium);
    SSLN_DEBUGF("2 This is a debug message with time in file or console with lite format");
    ssln::Logger::GetInstance()->SetVerbose(ssln::Logger::Verbose::kFull);
    SSLN_DEBUGF("3 This is a debug message with time in file or console with lite format");

    return 0;
}
