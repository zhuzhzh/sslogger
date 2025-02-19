#define QUILL_COMPILE_ACTIVE_LOG_LEVEL QUILL_COMPILE_ACTIVE_LOG_LEVEL_TRACE_L3
#include "ssln/sslogger.h"
#include "ssln/sslogger_macros.h"
#include "quill/StopWatch.h"
#include "quill/Frontend.h"
#include <gtest/gtest.h>
#include <sstream>
#include <thread>
#include <regex>

class StopwatchTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize console logger with minimal pattern for testing
        logger = ssln::SetupConsole(quill::LogLevel::Debug, ssln::Verbose::kLite, "stopwatch_test");
        ssln::set_default_logger(logger);
    }

    void TearDown() override {
        auto loggers = quill::Frontend::get_all_loggers();
        for (auto& logger : loggers) {
            quill::Frontend::remove_logger(logger);
        }
        ssln::default_logger = nullptr;
    }

    // Helper function to check if elapsed time is within expected range
    void CheckElapsedTime(const std::chrono::nanoseconds& elapsed, 
                         double expected_ms, double tolerance_ms = 20) {
        double actual_ms = std::chrono::duration<double, std::milli>(elapsed).count();
        EXPECT_NEAR(actual_ms, expected_ms, tolerance_ms)
            << "Time measurement outside tolerance range.\n"
            << "Expected: " << expected_ms << "ms ± " << tolerance_ms << "ms\n"
            << "Actual: " << actual_ms << "ms\n";
    }

    quill::Logger* logger;
};

TEST_F(StopwatchTest, TscBasicMeasurement) {
    quill::StopWatchTsc sw;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auto elapsed = sw.elapsed_as<std::chrono::nanoseconds>();
    CheckElapsedTime(elapsed, 100);
}

TEST_F(StopwatchTest, ChronoBasicMeasurement) {
    quill::StopWatchChrono sw;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auto elapsed = sw.elapsed_as<std::chrono::nanoseconds>();
    CheckElapsedTime(elapsed, 100);
}

TEST_F(StopwatchTest, TscMultipleIntervals) {
    quill::StopWatchTsc sw;
    
    // First interval
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto first = sw.elapsed_as<std::chrono::nanoseconds>();
    CheckElapsedTime(first, 50);
    
    // Second interval
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto second = sw.elapsed_as<std::chrono::nanoseconds>();
    CheckElapsedTime(second, 100);
}

TEST_F(StopwatchTest, ChronoReset) {
    quill::StopWatchChrono sw;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    sw.reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    auto elapsed = sw.elapsed_as<std::chrono::nanoseconds>();
    CheckElapsedTime(elapsed, 50);
}

TEST_F(StopwatchTest, DifferentUnits) {
    quill::StopWatchTsc sw;
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    
    // Check seconds
    auto sec = sw.elapsed_as<std::chrono::seconds>();
    EXPECT_EQ(sec.count(), 1);
    
    // Check milliseconds
    auto ms = sw.elapsed_as<std::chrono::milliseconds>();
    EXPECT_GE(ms.count(), 1500);
    EXPECT_LT(ms.count(), 1600);  // Allow some tolerance
    
    // Check microseconds
    auto us = sw.elapsed_as<std::chrono::microseconds>();
    EXPECT_GE(us.count(), 1500000);
    EXPECT_LT(us.count(), 1600000);
}

int main(int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}