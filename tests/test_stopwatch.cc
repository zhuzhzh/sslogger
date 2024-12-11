#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
// tests/test_stopwatch.cc
#include "ssln/sslogger.h"
#include <gtest/gtest.h>
#include <sstream>
#include <thread>
#include <regex>
#include <spdlog/sinks/ostream_sink.h>

class StopwatchTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_stream_ = std::make_shared<std::ostringstream>();
        auto ostream_sink = std::make_shared<spdlog::sinks::ostream_sink_st>(*test_stream_);
        auto logger = std::make_shared<spdlog::logger>("stopwatch_logger", ostream_sink);
        logger->set_level(spdlog::level::debug);
        logger->set_pattern("%v"); // Only message for easier testing
        spdlog::set_default_logger(logger);
    }

    void TearDown() override {
        spdlog::drop_all();
    }

    // Helper function to check if elapsed time is within expected range
    void check_elapsed_time(const std::string& output, double expected_ms, double tolerance_ms = 20) {
        std::regex time_pattern(R"([\d.]+)");
        std::smatch matches;
        ASSERT_TRUE(std::regex_search(output, matches, time_pattern)) 
            << "No time value found in output: " << output;

        double actual_ms = std::stod(matches[0]) * 1000; // Convert to milliseconds
        EXPECT_NEAR(actual_ms, expected_ms, tolerance_ms)
            << "Time measurement outside tolerance range.\n"
            << "Expected: " << expected_ms << "ms ± " << tolerance_ms << "ms\n"
            << "Actual: " << actual_ms << "ms\n"
            << "Full output: " << output;
    }

    std::shared_ptr<std::ostringstream> test_stream_;
};

TEST_F(StopwatchTest, BasicMeasurement) {
    spdlog::stopwatch sw;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Basic elapsed time
    test_stream_->str(""); // Clear previous output
    spdlog::info("Elapsed time: {}", sw);
    check_elapsed_time(test_stream_->str(), 100);
}

TEST_F(StopwatchTest, FormattedOutput) {
    spdlog::stopwatch sw;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Check formatted output with 3 decimal places
    test_stream_->str("");
    spdlog::info("Formatted time: {:.3}", sw);
    std::string output = test_stream_->str();
    
    // Verify format has exactly 3 decimal places
    std::regex format_pattern(R"(\d+\.\d{3})");
    EXPECT_TRUE(std::regex_search(output, format_pattern)) 
        << "Output doesn't match expected format (X.XXX): " << output;
    
    check_elapsed_time(output, 50);
}

TEST_F(StopwatchTest, MultipleIntervals) {
    spdlog::stopwatch sw;
    
    // First interval
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    test_stream_->str("");
    spdlog::info("First interval: {}", sw);
    check_elapsed_time(test_stream_->str(), 50);
    
    // Second interval
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    test_stream_->str("");
    spdlog::info("Second interval: {}", sw);
    check_elapsed_time(test_stream_->str(), 100);
}

TEST_F(StopwatchTest, ElapsedCount) {
    spdlog::stopwatch sw;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    test_stream_->str("");
    SPDLOG_INFO("Elapsed seconds: {} seconds", sw.elapsed().count());
    
    std::string output = test_stream_->str();
    check_elapsed_time(output, 100);
    
    // Verify "seconds" unit is present
    EXPECT_TRUE(output.find("seconds") != std::string::npos) 
        << "Output missing 'seconds' unit: " << output;
}

int main(int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}