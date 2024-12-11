#include "ssln/sslogger.h"

int main() {

    // 同步文件日志
    ssln::init_sync_file("logs", "sync.log", spdlog::level::debug, ssln::Verbose::kFull, "sync_logger");
    SPDLOG_INFO("Sync file logging");

    // 异步文件日志
    ssln::init_async_file("logs", "async.log", spdlog::level::debug, ssln::Verbose::kUltra, "async_logger");
    auto async_logger = spdlog::get("async_logger");
    spdlog::set_default_logger(async_logger);
    SPDLOG_INFO("Async file logging");

    ssln::init_async_file("logs", "async2.log", spdlog::level::debug, ssln::Verbose::kHigh, "async_logger2");
    auto async2_logger = spdlog::get("async_logger2");
    spdlog::set_default_logger(async2_logger);
    SPDLOG_INFO("Async file2 logging");

    // Rotating文件日志 (最大10MB，保留5个文件)
    ssln::init_rotating_file("logs", "rotate.log", 
        10 * 1024 * 1024, 5, spdlog::level::debug, ssln::Verbose::kLite);
    auto rotating_logger = spdlog::get("rotating_logger");
    spdlog::set_default_logger(rotating_logger);
    SPDLOG_INFO("Rotating file logging");

    spdlog::set_default_logger(async_logger);
    SPDLOG_INFO("Async file logging again");
    
    return 0;
}
