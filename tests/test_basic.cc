#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
// tests/test_basic.cc
#include "ssln/sslogger.h"
#include <gtest/gtest.h>
#include <sstream>
#include <regex>
#include <spdlog/sinks/ostream_sink.h>

class BasicLoggerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    test_stream_ = std::make_shared<std::ostringstream>();
    sink_ = std::make_shared<spdlog::sinks::ostream_sink_st>(*test_stream_);
  }

  void TearDown() override {
    spdlog::drop_all();
  }

  void CreateLogger(const std::string& name) {
    auto logger = std::make_shared<spdlog::logger>(name, sink_);
    logger->set_level(spdlog::level::debug);
    logger->set_pattern("%v");  // Only message for easier testing
    spdlog::set_default_logger(logger);
    test_stream_->str("");
  }

  void CheckOutput(const std::string& expected) {
    std::string output = test_stream_->str();
    EXPECT_FALSE(output.empty()) << "Log output should not be empty";
    EXPECT_TRUE(output.find(expected) != std::string::npos)
        << "Expected message not found.\n"
        << "Expected: " << expected << "\n"
        << "Actual: " << output;
  }

  std::shared_ptr<std::ostringstream> test_stream_;
  std::shared_ptr<spdlog::sinks::ostream_sink_st> sink_;
};

TEST_F(BasicLoggerTest, ConsoleLogger) {
  ssln::InitConsole(spdlog::level::debug, ssln::Verbose::kLite, "console_test");
  auto logger = spdlog::get("console_test");
  ASSERT_TRUE(logger != nullptr) << "Failed to get console logger";
  EXPECT_EQ(logger->level(), spdlog::level::debug);
}

TEST_F(BasicLoggerTest, SyncFileLogger) {
  ssln::InitSyncFile("logs", "test.log", spdlog::level::info,
                     ssln::Verbose::kMedium, "sync_test");
  auto logger = spdlog::get("sync_test");
  ASSERT_TRUE(logger != nullptr) << "Failed to get sync file logger";
  EXPECT_EQ(logger->level(), spdlog::level::info);
}

TEST_F(BasicLoggerTest, AsyncFileLogger) {
  ssln::InitAsyncFile("logs", "test_async.log", spdlog::level::warn,
                      ssln::Verbose::kHigh, "async_test");
  auto logger = spdlog::get("async_test");
  ASSERT_TRUE(logger != nullptr) << "Failed to get async file logger";
  EXPECT_EQ(logger->level(), spdlog::level::warn);
}

TEST_F(BasicLoggerTest, RotatingFileLogger) {
  ssln::InitRotatingFile("logs", "test_rotating.log", 1024 * 1024, 5,
                        spdlog::level::err, ssln::Verbose::kFull,
                        "rotating_test");
  auto logger = spdlog::get("rotating_test");
  ASSERT_TRUE(logger != nullptr) << "Failed to get rotating file logger";
  EXPECT_EQ(logger->level(), spdlog::level::err);
}

TEST_F(BasicLoggerTest, LogLevels) {
  CreateLogger("level_test");
  
  SPDLOG_TRACE("Trace message");  // Should not appear
  SPDLOG_DEBUG("Debug message");
  SPDLOG_INFO("Info message");
  SPDLOG_WARN("Warning message");
  SPDLOG_ERROR("Error message");
  SPDLOG_CRITICAL("Critical message");
  
  std::string output = test_stream_->str();
  EXPECT_EQ(output.find("Trace message"), std::string::npos)
      << "Trace message should not appear";
  EXPECT_NE(output.find("Debug message"), std::string::npos);
  EXPECT_NE(output.find("Info message"), std::string::npos);
  EXPECT_NE(output.find("Warning message"), std::string::npos);
  EXPECT_NE(output.find("Error message"), std::string::npos);
  EXPECT_NE(output.find("Critical message"), std::string::npos);
}

TEST_F(BasicLoggerTest, Stopwatch) {
  CreateLogger("stopwatch_test");
  ssln::Stopwatch sw;
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  
  SPDLOG_INFO("Time elapsed: {}", sw);
  std::string output = test_stream_->str();
  
  std::regex time_pattern(R"(\d+\.\d+)");
  EXPECT_TRUE(std::regex_search(output, time_pattern))
      << "No elapsed time found in output: " << output;
}

TEST_F(BasicLoggerTest, DuplicateLoggerNames) {
  EXPECT_NO_THROW(ssln::InitConsole(spdlog::level::info, ssln::Verbose::kLite,
                                   "unique_logger"));
  EXPECT_THROW(ssln::InitConsole(spdlog::level::info, ssln::Verbose::kLite,
                                "unique_logger"),
               std::runtime_error);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
