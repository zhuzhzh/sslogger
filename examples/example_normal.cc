#include "ssln/sslogger_macros.h"
#include "ssln/sslogger.h"
#include "quill/LogMacros.h"

int main() {

    // 同步文件日志
    ssln::SetupConsole();
    SSLN_INFO("console info");

    auto file_logger = ssln::SetupFile("log/normal.log", quill::LogLevel::TraceL1, ssln::Verbose::kMedium, "file_logger");
    SSLN_LOG_DEBUG(file_logger,"file Debug");

    // Rotating文件日志 (最大10MB，保留5个文件)
    auto rotate_logger = ssln::SetupRotatingFile("log/rotate.log",
        1 * 1024 * 1024, 5, quill::LogLevel::Debug, ssln::Verbose::kUltra);
    SSLN_LOG_INFO(rotate_logger, "Rotating info");
    SSLN_LOG_DEBUG(rotate_logger, "Rotating debug");

    
    return 0;
}
