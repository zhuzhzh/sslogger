// tests/test_basic.cc
#include "ssln/sslogger.h"
#include <gtest/gtest.h>

TEST(LoggerTest, BasicConsoleLogging) {
    // 初始化控制台日志
    ssln::init_console("console", spdlog::level::debug, ssln::Verbose::kMedium);
    
    // 基本日志测试
    spdlog::info("Basic console logging test");
    SPDLOG_DEBUG("Debug message with source info");
}

TEST(LoggerTest, FileLogging) {
    // 初始化文件日志
    ssln::init_sync_file("file_logger", "logs", "test.log", 
        spdlog::level::debug, ssln::Verbose::kFull);
    
    auto logger = spdlog::get("file_logger");
    ASSERT_TRUE(logger != nullptr);
    
    spdlog::set_default_logger(logger);
    SPDLOG_INFO("File logging test");
}

TEST(LoggerTest, AsyncFileLogging) {
    ssln::init_async_file("async_logger", "logs", "async_test.log", 
        spdlog::level::debug, ssln::Verbose::kFull);
    
    auto logger = spdlog::get("async_logger");
    ASSERT_TRUE(logger != nullptr);
    
    spdlog::set_default_logger(logger);
    SPDLOG_INFO("Async file logging test");
}

int main(int argc, char const *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
