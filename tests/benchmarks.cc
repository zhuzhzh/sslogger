#include "vhlogger/vhlogger.h"

int main() {
    vgp::Logger* logger = vgp::Logger::GetInstance();

    logger->SetVerbose(vgp::Logger::Verbose::kMedium);
    logger->SetLevel(spdlog::level::debug);
    logger->SetFile("bench.log");
    for (int i=0;i<1000000;i++) {
        VGP_INFOF("1 This is a debug message with time in file or console with lite format");
    }
    return 0;
}
