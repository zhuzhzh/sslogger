// tests/test_basic.cc
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
  ASSERT_TRUE(ssln::console_logger != nullptr) << "Failed to get console logger";
  EXPECT_EQ(ssln::console_logger->get_log_level(), quill::LogLevel::Info);
}

TEST_F(BasicLoggerTest, AsyncFileLogger) {
  ASSERT_TRUE(ssln::hybrid_logger != nullptr) << "Failed to get async file logger";
  //file_logger->set_log_level(quill::LogLevel::Info);
  EXPECT_EQ(ssln::hybrid_logger->get_log_level(), quill::LogLevel::Info);

  // Test logging
  SSLN_LOG_INFO(ssln::hybrid_logger,"Test async file message");
  ssln::hybrid_logger->flush_log();
  std::string log_file = ssln::detail::GetLoggerFilePath("hybrid_logger");
  EXPECT_TRUE(ContainsString(log_file, "Test async file message"));
}

TEST_F(BasicLoggerTest, RotatingFileLogger) {
  ASSERT_TRUE(ssln::daily_logger != nullptr) << "Failed to get rotating file logger";
  //rotating_logger->set_log_level(quill::LogLevel::Warning);
  EXPECT_EQ(ssln::daily_logger->get_log_level(), quill::LogLevel::Info);

  // Test logging
  SSLN_LOG_WARNING(ssln::daily_logger,"Test rotating file message");
  ssln::daily_logger->flush_log();
  //EXPECT_TRUE(ContainsString("daily.log", "Test rotating file message"));
}

TEST_F(BasicLoggerTest, LogLevels) {
  ssln::set_default_logger(ssln::console_logger);
  ssln::console_logger->set_log_level(quill::LogLevel::Debug);

  SSLN_TRACE_L3("Trace message");  // Should not appear
  SSLN_DEBUG("Debug message");
  SSLN_INFO("Info message");
  SSLN_WARNING("Warning message");
  SSLN_ERROR("Error message");
  SSLN_CRITICAL("Critical message");
}

TEST_F(BasicLoggerTest, DefaultLogger) {
  // First logger becomes default
  ssln::set_default_logger(ssln::hybrid_logger);
  EXPECT_EQ(ssln::default_logger, ssln::hybrid_logger);

  // Second logger should not become default
  ssln::set_default_logger(ssln::console_logger);
  EXPECT_EQ(ssln::default_logger, ssln::console_logger);

  // Explicitly set default logger
  ssln::set_default_logger(ssln::daily_logger);
  EXPECT_EQ(ssln::default_logger, ssln::daily_logger);
}

TEST_F(BasicLoggerTest, GetLogger) {
  auto retrieved = ssln::get_logger("hybrid_logger");
  EXPECT_EQ(ssln::hybrid_logger, retrieved);

  // Non-existent logger
  //EXPECT_EQ(ssln::get_logger("non_existent"), nullptr);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
