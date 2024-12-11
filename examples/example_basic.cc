#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#include "ssln/sslogger.h"

int main() {

    SPDLOG_DEBUG("debug message");
    SPDLOG_CRITICAL("critical message");
    SPDLOG_TRACE("trace message");
    SPDLOG_INFO("info message");
    SPDLOG_WARN("warning message");
    SPDLOG_ERROR("error message");

    spdlog::set_level(spdlog::level::debug);
    spdlog::debug("this debug will show");
    spdlog::trace("this trace will not show");
    return 0;
}