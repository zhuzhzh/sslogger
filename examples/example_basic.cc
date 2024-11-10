#include "ssln/sslogger.h"

using ssln::g_logger;

int main(int argc, char* argv[]) {
    // 初始化日志系统
    ssln::Logger::Init(".", "hybrid.log", false, SSLOGGER_INFO);

    // 使用日志
    SSLN_INFO("Program started");
    SSLN_INFOF("Program started");
    
    // 可以在程序中间更改日志文件
    g_logger->SetFile("./another.log");
    SSLN_INFO("Writing to another file");
    SSLN_INFOF("Writing to another file");

    // 程序结束前关闭日志系统
    ssln::Logger::Shutdown();
    return 0;
}
