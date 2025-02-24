#include "ssln/sslogger.h"
#include "quill/LogMacros.h"

int main() {

    // 同步文件日志
    ssln::SetupConsoleLogger("console_logger", ssln::Verbose::kMedium, quill::LogLevel::Debug);
    ssln::set_default_logger("console_logger");
    SSLN_INFO("console info");

    auto file_logger = ssln::SetupFileLogger("log/normal.log", "file_logger", ssln::Verbose::kMedium, quill::LogLevel::Debug, true);
    SSLN_LOG_DEBUG(file_logger,"file Debug");

    // Rotating文件日志 (最大10MB，保留5个文件)
    auto rotate_logger = ssln::SetupRotatingLogger("log/rotate.log",
        1 * 1024 * 1024, 5, "rotate_logger", ssln::Verbose::kUltra);
    SSLN_LOG_INFO(rotate_logger, "Rotating info");
    SSLN_LOG_DEBUG(rotate_logger, "Rotating debug");

    
    return 0;
}
