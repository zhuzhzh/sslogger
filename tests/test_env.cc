#include <gtest/gtest.h>
#include "ssln/sslogger.h"
#include <cstdlib>

using namespace ssln;

class LoggerEnvTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Store original env value if exists
        const char* original_level = std::getenv("SSLN_LOG_LEVEL");
        if (original_level) {
            original_log_level_ = original_level;
        }
        
        // Initialize logger if not already initialized
        if (!g_logger) {
            Logger::Init(".", "", false, SSLOGGER_INFO, Logger::Verbose::kMedium, true);
        }
    }

    void TearDown() override {
        // Restore original environment variable
        if (!original_log_level_.empty()) {
            setenv("SSLN_LOG_LEVEL", original_log_level_.c_str(), 1);
        } else {
            unsetenv("SSLN_LOG_LEVEL");
        }
        
        // Reload environment variables to restore original state
        if (g_logger) {
            g_logger->ReloadFromEnv();
        }
    }

    std::string original_log_level_;
};

// Test default initialization
TEST_F(LoggerEnvTest, DefaultLogLevel) {
    unsetenv("SSLN_LOG_LEVEL");
    g_logger->ReloadFromEnv();
    EXPECT_EQ(g_logger->GetCurrentLogger()->level(), SSLOGGER_INFO);
}

// Test DEBUG level
TEST_F(LoggerEnvTest, DebugLogLevel) {
    setenv("SSLN_LOG_LEVEL", "DEBUG", 1);
    g_logger->ReloadFromEnv();
    EXPECT_EQ(g_logger->GetCurrentLogger()->level(), SSLOGGER_DEBUG);
}

// Test INFO level
TEST_F(LoggerEnvTest, InfoLogLevel) {
    setenv("SSLN_LOG_LEVEL", "INFO", 1);
    g_logger->ReloadFromEnv();
    EXPECT_EQ(g_logger->GetCurrentLogger()->level(), SSLOGGER_INFO);
}

// Test WARN level
TEST_F(LoggerEnvTest, WarnLogLevel) {
    setenv("SSLN_LOG_LEVEL", "WARN", 1);
    g_logger->ReloadFromEnv();
    EXPECT_EQ(g_logger->GetCurrentLogger()->level(), SSLOGGER_WARN);
}

// Test ERROR level
TEST_F(LoggerEnvTest, ErrorLogLevel) {
    setenv("SSLN_LOG_LEVEL", "ERROR", 1);
    g_logger->ReloadFromEnv();
    EXPECT_EQ(g_logger->GetCurrentLogger()->level(), SSLOGGER_ERROR);
}

// Test FATAL level
TEST_F(LoggerEnvTest, FatalLogLevel) {
    setenv("SSLN_LOG_LEVEL", "FATAL", 1);
    g_logger->ReloadFromEnv();
    EXPECT_EQ(g_logger->GetCurrentLogger()->level(), SSLOGGER_FATAL);
}

// Test invalid log level
TEST_F(LoggerEnvTest, InvalidLogLevel) {
    setenv("SSLN_LOG_LEVEL", "INVALID_LEVEL", 1);
    g_logger->ReloadFromEnv();
    // Should default to INFO when invalid
    EXPECT_EQ(g_logger->GetCurrentLogger()->level(), SSLOGGER_INFO);
}

// Test numeric log levels
TEST_F(LoggerEnvTest, NumericLogLevels) {
    // Test all numeric levels
    const std::pair<const char*, spdlog::level::level_enum> numeric_levels[] = {
        {"0", SSLOGGER_TRACE},
        {"1", SSLOGGER_DEBUG},
        {"2", SSLOGGER_INFO},
        {"3", SSLOGGER_WARN},
        {"4", SSLOGGER_ERROR},
        {"5", SSLOGGER_FATAL},
        {"6", SSLOGGER_OFF}
    };

    for (const auto& [level_str, expected_level] : numeric_levels) {
        setenv("SSLN_LOG_LEVEL", level_str, 1);
        g_logger->ReloadFromEnv();
        EXPECT_EQ(g_logger->GetCurrentLogger()->level(), expected_level)
            << "Failed for numeric level: " << level_str;
    }
}

// Test case sensitivity
TEST_F(LoggerEnvTest, LogLevelCaseInsensitive) {
    const std::vector<std::string> debug_variants = {
        "DEBUG", "Debug", "debug", "DeBuG"
    };

    for (const auto& variant : debug_variants) {
        setenv("SSLN_LOG_LEVEL", variant.c_str(), 1);
        g_logger->ReloadFromEnv();
        EXPECT_EQ(g_logger->GetCurrentLogger()->level(), SSLOGGER_DEBUG)
            << "Failed for variant: " << variant;
    }
}

// Test environment variable override disabled
TEST_F(LoggerEnvTest, EnvOverrideDisabled) {
    Logger::Shutdown();
    // Reinitialize logger with env override disabled
    Logger::Init(".", "", false, SSLOGGER_INFO, Logger::Verbose::kMedium, false);
    
    setenv("SSLN_LOG_LEVEL", "DEBUG", 1);
    g_logger->ReloadFromEnv();
    // Should remain at INFO level since env override is disabled
    EXPECT_EQ(g_logger->GetCurrentLogger()->level(), SSLOGGER_INFO);
}