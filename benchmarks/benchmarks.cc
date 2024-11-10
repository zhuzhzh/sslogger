#include "ssln/sslogger.h"

int main() {
    ssln::Logger* logger = ssln::Logger::GetInstance();

    logger->SetVerbose(ssln::Logger::Verbose::kMedium);
    logger->SetLevel(spdlog::level::debug);
    logger->SetFile("bench.log");
    for (int i=0;i<1000000;i++) {
        SSLN_INFOF("1 This is a debug message with time in file or console with lite format");
    }
    return 0;
}
