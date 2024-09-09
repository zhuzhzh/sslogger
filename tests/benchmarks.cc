#include "vhlogger/vhlogger.h"

int main() {
    vgp::Logger* logger = vgp::Logger::GetInstance();

    logger->SetFormat(vgp::Logger::Format::kMedium);
    logger->SetLogLevel(spdlog::level::debug);
    logger->SetLogFile("bench.log");
    for (int i=0;i<1000000;i++) {
        VGP_LOGFI("1 This is a debug message with time in file or console with lite format");
    }
    return 0;
}
