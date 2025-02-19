#define QUILL_COMPILE_ACTIVE_LOG_LEVEL QUILL_COMPILE_ACTIVE_LOG_LEVEL_TRACE_L3
// tests/test_basic.cc
#include "ssln/sslogger_macros.h"
#include "ssln/sslogger.h"

// C++ standard library headers
#include <fstream>
#include <sstream>
#include <regex>
#include <thread>

// Google Test headers
#include <gtest/gtest.h>

// Quill headers
#include "quill/Backend.h"
#include "quill/Frontend.h"
#include "quill/LogMacros.h"
#include "quill/Logger.h"
#include "quill/sinks/ConsoleSink.h"
#include "quill/sinks/FileSink.h"
#include "quill/sinks/RotatingFileSink.h"

class BasicLoggerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Initialize backend
    //ssln::InitBackend();
  }

  void TearDown() override {
    // Reset all loggers
    auto loggers = quill::Frontend::get_all_loggers();
    for (auto& logger : loggers) {
      quill::Frontend::remove_logger(logger);
    }
    ssln::default_logger = nullptr;
    while(quill::Frontend::get_number_of_loggers() != 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }

  // Helper function to check if output contains expected string
  bool ContainsString(const std::string& filename, const std::string& expected) {
    std::ifstream file(filename.c_str(), std::ifstream::out);
    if (!file.is_open()) {
      ADD_FAILURE() << "Could not open log file: " << filename;
      return false;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    file.close();

    bool contains = content.find(expected) != std::string::npos;
    if (!contains) {
      ADD_FAILURE() << "Expected message not found.\n"
                   << "Expected: " << expected << "\n"
                   << "Log content: " << content;
    }
    return contains;
  }
};

TEST_F(BasicLoggerTest, ConsoleLogger) {
  auto logger = ssln::SetupConsole(quill::LogLevel::Debug, ssln::Verbose::kLite, "console_test");
  ASSERT_TRUE(logger != nullptr) << "Failed to get console logger";
  EXPECT_EQ(logger->get_log_level(), quill::LogLevel::Debug);
}

TEST_F(BasicLoggerTest, AsyncFileLogger) {
  auto file_logger = ssln::SetupFile("test_async.log", quill::LogLevel::Info,
                                  ssln::Verbose::kMedium, "async_test");
  ASSERT_TRUE(file_logger != nullptr) << "Failed to get async file logger";
  //file_logger->set_log_level(quill::LogLevel::Info);
  std::cout << "File logger log level: " << static_cast<int>(file_logger->get_log_level()) << std::endl;
  EXPECT_EQ(file_logger->get_log_level(), quill::LogLevel::Info);

  // Test logging
  SSLN_LOG_INFO(file_logger,"Test async file message");
  file_logger->flush_log();
  EXPECT_TRUE(ContainsString("test_async.log", "Test async file message"));
}

TEST_F(BasicLoggerTest, RotatingFileLogger) {
  auto rotating_logger = ssln::SetupRotatingFile("test_rotating.log", 1024 * 1024, 5,
                                     quill::LogLevel::Warning, ssln::Verbose::kFull,
                                     "rotating_test");
  ASSERT_TRUE(rotating_logger != nullptr) << "Failed to get rotating file logger";
  //rotating_logger->set_log_level(quill::LogLevel::Warning);
  std::cout << "Rotating logger log level: " << static_cast<int>(rotating_logger->get_log_level()) << std::endl;
  EXPECT_EQ(rotating_logger->get_log_level(), quill::LogLevel::Warning);

  // Test logging
  SSLN_LOG_WARNING(rotating_logger,"Test rotating file message");
  rotating_logger->flush_log();
  EXPECT_TRUE(ContainsString("test_rotating.log", "Test rotating file message"));
}

TEST_F(BasicLoggerTest, LogLevels) {
  auto basic_logger = ssln::SetupConsole(quill::LogLevel::Debug, ssln::Verbose::kLite, "level_test");
  ssln::set_default_logger(basic_logger);

  SSLN_TRACE_L3("Trace message");  // Should not appear
  SSLN_DEBUG("Debug message");
  SSLN_INFO("Info message");
  SSLN_WARNING("Warning message");
  SSLN_ERROR("Error message");
  SSLN_CRITICAL("Critical message");
}

TEST_F(BasicLoggerTest, DefaultLogger) {
  // First logger becomes default
  auto logger1 = ssln::SetupConsole(quill::LogLevel::Info, ssln::Verbose::kLite, "default_test1");
  EXPECT_EQ(ssln::default_logger, logger1);

  // Second logger should not become default
  auto logger2 = ssln::SetupConsole(quill::LogLevel::Debug, ssln::Verbose::kLite, "default_test2");
  EXPECT_EQ(ssln::default_logger, logger1);

  // Explicitly set default logger
  ssln::set_default_logger(logger2);
  EXPECT_EQ(ssln::default_logger, logger2);
}

TEST_F(BasicLoggerTest, GetLogger) {
  auto logger = ssln::SetupConsole(quill::LogLevel::Info, ssln::Verbose::kLite, "get_test");
  auto retrieved = ssln::get_logger("get_test");
  EXPECT_EQ(logger, retrieved);

  // Non-existent logger
  EXPECT_EQ(ssln::get_logger("non_existent"), nullptr);
}

TEST_F(BasicLoggerTest, DuplicateLoggerNames) {
  EXPECT_NO_THROW(ssln::SetupConsole(quill::LogLevel::Info, ssln::Verbose::kLite,
                                  "unique_logger"));
  // Getting existing logger should not throw
  EXPECT_NO_THROW(ssln::SetupConsole(quill::LogLevel::Info, ssln::Verbose::kLite,
                                  "unique_logger"));
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
