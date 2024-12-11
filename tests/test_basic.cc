#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
// tests/test_basic.cc
#include "ssln/sslogger.h"
#include <gtest/gtest.h>

TEST(LoggerTest, BasicConsoleLogging) {
    // 初始化控制台日志
    ssln::init_console(spdlog::level::debug, ssln::Verbose::kMedium, "console");
    
    // 基本日志测试
    spdlog::info("Basic console logging test");
    SPDLOG_DEBUG("Debug message with source info");
}

TEST(LoggerTest, FileLogging) {
    // 初始化文件日志
    ssln::init_sync_file("logs", "test.log", 
        spdlog::level::debug, ssln::Verbose::kFull, "file_logger");
    
    auto logger = spdlog::get("file_logger");
    ASSERT_TRUE(logger != nullptr);
    
    spdlog::set_default_logger(logger);
    SPDLOG_INFO("File logging test");
}

TEST(LoggerTest, AsyncFileLogging) {
    ssln::init_async_file("logs", "async_test.log", 
        spdlog::level::debug, ssln::Verbose::kFull, "async_logger");
    
    auto logger = spdlog::get("async_logger");
    ASSERT_TRUE(logger != nullptr);
    
    spdlog::set_default_logger(logger);
    SPDLOG_INFO("Async file logging test");
}

int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
