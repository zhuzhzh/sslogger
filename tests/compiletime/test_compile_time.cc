#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../../include/ssln/sslogger.h"
#include <sstream>
#include <regex>

class SSLoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 重置日志实例
        ssln::Logger::GetInstance()->SetLevel(SSLOGGER_TRACE);
        ssln::Logger::GetInstance()->SetVerbose(ssln::Logger::Verbose::kMedium);
        ssln::Logger::GetInstance()->SetAsyncMode(false);
    }

    void TearDown() override {
        // 清理工作（如果需要）
    }

    // 辅助函数：捕获日志输出
    std::string CaptureLog(std::function<void()> logFunc) {
        testing::internal::CaptureStdout();
        logFunc();
        std::string output = testing::internal::GetCapturedStdout();
        std::cout << "Captured Log Output:\n" << output << std::endl;
        return output;
    }

   // 辅助函数：检查日志输出是否匹配预期模式
    bool LogMatches(const std::string& log, const std::string& level, const std::string& message) {
        char level_char = std::toupper(level[0]);
        std::string pattern_str = "\\[\\d{2}:\\d{2}:\\d{2}\\.\\d{6}\\]\\[" + std::string(1, level_char) + "\\]\\[.+\\] " + message;
        std::regex pattern(pattern_str);
        bool match = std::regex_search(log, pattern);
        if (!match) {
            std::cout << "Failed to match log: " << log << std::endl;
            std::cout << "Expected pattern: " << pattern_str << std::endl;
        }
        return match;
    } 

};

TEST_F(SSLoggerTest, BasicLogging) {
    auto output = CaptureLog([]() {
        SSLN_TRACE("This is a trace message");
        SSLN_DEBUG("This is a debug message");
        SSLN_INFO("This is an info message");
        SSLN_WARN("This is a warning message");
        SSLN_ERROR("This is an error message");
        SSLN_FATAL("This is a fatal message");
    });

    EXPECT_TRUE(LogMatches(output, "trace", "This is a trace message"))
        << "Failed to match trace message. Output: " << output;
    EXPECT_TRUE(LogMatches(output, "debug", "This is a debug message"))
        << "Failed to match debug message. Output: " << output;
    EXPECT_TRUE(LogMatches(output, "info", "This is an info message"))
        << "Failed to match info message. Output: " << output;
    EXPECT_TRUE(LogMatches(output, "warning", "This is a warning message"))
        << "Failed to match warning message. Output: " << output;
    EXPECT_TRUE(LogMatches(output, "error", "This is an error message"))
        << "Failed to match error message. Output: " << output;
    EXPECT_TRUE(LogMatches(output, "critical", "This is a fatal message"))
        << "Failed to match fatal message. Output: " << output;
}

TEST_F(SSLoggerTest, CompileTimeLogging) {
    auto output = CaptureLog([]() {
        SSLN_TRACE_CT("This is a compile-time trace message");
        SSLN_DEBUG_CT("This is a compile-time debug message");
        SSLN_INFO_CT("This is a compile-time info message");
        SSLN_WARN_CT("This is a compile-time warning message");
        SSLN_ERROR_CT("This is a compile-time error message");
        SSLN_FATAL_CT("This is a compile-time fatal message");
    });

    EXPECT_FALSE(LogMatches(output, "trace", "This is a compile-time trace message"));
    EXPECT_FALSE(LogMatches(output, "debug", "This is a compile-time debug message"));
    EXPECT_TRUE(LogMatches(output, "info", "This is a compile-time info message"));
    EXPECT_TRUE(LogMatches(output, "warning", "This is a compile-time warning message"));
    EXPECT_TRUE(LogMatches(output, "error", "This is a compile-time error message"));
    EXPECT_TRUE(LogMatches(output, "critical", "This is a compile-time fatal message"));
}

TEST_F(SSLoggerTest, LogLevelControl) {
    ssln::Logger::GetInstance()->SetLevel(SSLOGGER_DEBUG);
    auto output = CaptureLog([]() {
        SSLN_TRACE("This should not be logged");
        SSLN_DEBUG("This should be logged");
        SSLN_INFO("This should be logged");
        SSLN_WARN("This should be logged");
        SSLN_ERROR("This should be logged");
        SSLN_FATAL("This should be logged");
    });

    EXPECT_FALSE(LogMatches(output, "trace", "This should not be logged"));
    EXPECT_TRUE(LogMatches(output, "debug", "This should be logged"));
    EXPECT_TRUE(LogMatches(output, "info", "This should be logged"));
    EXPECT_TRUE(LogMatches(output, "warning", "This should be logged"));
    EXPECT_TRUE(LogMatches(output, "error", "This should be logged"));
    EXPECT_TRUE(LogMatches(output, "critical", "This should be logged"));
}

TEST_F(SSLoggerTest, CompileTimeLogLevelControl) {
    // 注意：这个测试的行为取决于 SSLN_ACTIVE_LEVEL 的定义
    // 假设 SSLN_ACTIVE_LEVEL 被设置为 SSLOGGER_INFO
    auto output = CaptureLog([]() {
        SSLN_TRACE_CT("This should not be compiled");
        SSLN_DEBUG_CT("This should not be compiled");
        SSLN_INFO_CT("This should be logged");
        SSLN_WARN_CT("This should be logged");
        SSLN_ERROR_CT("This should be logged");
        SSLN_FATAL_CT("This should be logged");
    });

    EXPECT_FALSE(LogMatches(output, "trace", "This should not be compiled"));
    EXPECT_FALSE(LogMatches(output, "debug", "This should not be compiled"));
    EXPECT_TRUE(LogMatches(output, "info", "This should be logged"));
    EXPECT_TRUE(LogMatches(output, "warning", "This should be logged"));
    EXPECT_TRUE(LogMatches(output, "error", "This should be logged"));
    EXPECT_TRUE(LogMatches(output, "critical", "This should be logged"));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
