// tests/test_env.cc
#include "ssln/sslogger.h"

// C++ standard library headers
#include <fstream>
#include <sstream>
#include <regex>
#include <thread>
#include <cstdlib>
#include <filesystem>

// Google Test headers
#include <gtest/gtest.h>

// Quill headers
#include "quill/Frontend.h"

class EnvTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Save original environment variables if they exist
        SaveEnvVar("SSLN_LOG_LEVEL");
        SaveEnvVar("SSLN_VERBOSITY");
        SaveEnvVar("SSLN_LOG_DIR");
        SaveEnvVar("SSLN_LOG_NAME");
    }

    void TearDown() override {
        // Restore original environment variables
        RestoreEnvVar("SSLN_LOG_LEVEL");
        RestoreEnvVar("SSLN_VERBOSITY");
        RestoreEnvVar("SSLN_LOG_DIR");
        RestoreEnvVar("SSLN_LOG_NAME");

        // Reset all loggers
        auto loggers = quill::Frontend::get_all_loggers();
        for (auto& logger : loggers) {
            quill::Frontend::remove_logger(logger);
        }
        ssln::default_logger = nullptr;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    void SaveEnvVar(const char* name) {
        const char* value = std::getenv(name);
        if (value) {
            saved_env_[name] = value;
        }
    }

    void RestoreEnvVar(const char* name) {
        auto it = saved_env_.find(name);
        if (it != saved_env_.end()) {
#ifdef _WIN32
            _putenv_s(name, it->second.c_str());
#else
            setenv(name, it->second.c_str(), 1);
#endif
        } else {
#ifdef _WIN32
            _putenv_s(name, "");
#else
            unsetenv(name);
#endif
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

    std::map<std::string, std::string> saved_env_;
};

TEST_F(EnvTest, LogLevelEnv) {
#ifdef _WIN32
    _putenv_s("SSLN_LOG_LEVEL", "error");
#else
    setenv("SSLN_LOG_LEVEL", "error", 1);
#endif

    auto logger = ssln::SetupConsoleLogger("env_level_test", ssln::Verbose::kLite, quill::LogLevel::Info);
    ASSERT_TRUE(logger != nullptr);
    EXPECT_EQ(logger->get_log_level(), quill::LogLevel::Error);

    // Info messages should not appear
    SSLN_LOG_INFO(logger, "This should not appear");
    // Error messages should appear
    SSLN_LOG_ERROR(logger, "This should appear");
}

TEST_F(EnvTest, VerbosityEnv) {
#ifdef _WIN32
    _putenv_s("SSLN_VERBOSITY", "full");
#else
    setenv("SSLN_VERBOSITY", "full", 1);
#endif

    auto logger = ssln::SetupConsoleLogger("env_verbose_test", ssln::Verbose::kFull, quill::LogLevel::Info);
    ASSERT_TRUE(logger != nullptr);
    
    std::string pattern = ssln::GetPattern(ssln::Verbose::kFull);
    EXPECT_TRUE(std::regex_search(pattern, std::regex("%\\(caller_function\\)")))
        << "Full format missing function name: " << pattern;
}

TEST_F(EnvTest, LogFilePathEnv) {
#ifdef _WIN32
    _putenv_s("SSLN_LOG_DIR", "test_logs");
    _putenv_s("SSLN_LOG_NAME", "env_test.log");
#else
    setenv("SSLN_LOG_DIR", "test_logs", 1);
    setenv("SSLN_LOG_NAME", "env_test.log", 1);
#endif

    // Create test directory
    std::filesystem::create_directories("test_logs");

    auto logger = ssln::SetupFileLogger("default.log", "env_file_test", 
                               ssln::Verbose::kLite, quill::LogLevel::Info);
    ASSERT_TRUE(logger != nullptr);

    SSLN_LOG_INFO(logger, "Test message");
    logger->flush_log();

    EXPECT_TRUE(ContainsString("test_logs/env_test.log", "Test message"));
}

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 