// tests/test_stopwatch.cc
#include "ssln/sslogger.h"
#include <gtest/gtest.h>
#include <thread>

TEST(LoggerTest, StopwatchBasic) {
    ssln::init_console(spdlog::level::debug, ssln::Verbose::kMedium, "stopwatch_logger");
    
    spdlog::stopwatch sw;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 基本计时
    spdlog::info("Elapsed time: {}", sw);
    
    // 格式化输出
    spdlog::info("Formatted time: {:.3}", sw);
    
    // 带源文件信息
    SPDLOG_INFO("With source info: {} seconds", sw.elapsed().count());
}

int main(int argc, char const *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}