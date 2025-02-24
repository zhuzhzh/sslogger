#include "ssln/sslogger.h"
#include "quill/LogMacros.h"

int main() {

    quill::Logger* basic_logger = ssln::get_logger("axi_master");
    ssln::set_default_logger(basic_logger);

    SSLN_TRACE_L3("trace L3 message - not shown");
    SSLN_TRACE_L2("trace L2 message - not shown");
    SSLN_TRACE_L1("trace L1 message - not shown");
    SSLN_DEBUG("debug message - not shown");
    SSLN_INFO("info message - shown");
    SSLN_WARNING("warning message - shown");
    SSLN_ERROR("error message - shown");

    basic_logger->set_log_level(quill::LogLevel::TraceL3);
    SSLN_TRACE_L3("trace L3 message2 - shown");
    SSLN_TRACE_L2("trace L2 message2 - shown");
    SSLN_TRACE_L1("trace L1 message2 - shown");
    SSLN_DEBUG("debug message2 - shown");

    ssln::set_default_logger(ssln::console_logger);

    SSLN_TRACE_L3("trace L3 message - not shown");
    SSLN_TRACE_L2("trace L2 message - not shown");
    SSLN_TRACE_L1("trace L1 message - not shown");
    SSLN_DEBUG("debug message - not shown");
    SSLN_INFO("info message - shown");
    SSLN_WARNING("warning message - shown");
    SSLN_ERROR("error message - shown");

    ssln::console_logger->set_log_level(quill::LogLevel::TraceL3);

    SSLN_TRACE_L3("trace L3 message - shown");
    SSLN_TRACE_L2("trace L2 message - shown");
    SSLN_TRACE_L1("trace L1 message - shown");
    SSLN_DEBUG("debug message - shown");

    SSLN_LOG_DEBUG(ssln::console_logger, "{:>30}", "abcdefg");
    SSLN_LOG_DEBUG(ssln::console_logger, "{:#04x}", 48);
    SSLN_LOG_DEBUG(ssln::console_logger, "{:.2f}", 48.0);

    return 0;
}
