#include "vhlogger/vhlogger.h"

int main() {

    //vgp::Logger::Init();

    vgp::Logger* logger = vgp::Logger::GetInstance();

    logger->SetFormat(vgp::Logger::Format::kLite);
    logger->SetLogLevel(LOGLEVEL_DEBUG);
    logger->SetLogFile("bench.log");
    for (int i=0;i<1000000;i++) {
        VGP_DEBUGF("1 This is a debug message with time in file or console with lite format");
    }


    return 0;
}
