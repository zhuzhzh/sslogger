#define QUILL_COMPILE_ACTIVE_LOG_LEVEL QUILL_COMPILE_ACTIVE_LOG_LEVEL_TRACE_L3
// tests/test_verbosity.cc
#include "ssln/sslogger.h"
#include "ssln/sslogger_macros.h"

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

class VerbosityTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize backend
        ssln::InitBackend();
    }

    void TearDown() override {
        auto loggers = quill::Frontend::get_all_loggers();
        for (auto& logger : loggers) {
            quill::Frontend::remove_logger(logger);
        }
        while(quill::Frontend::get_number_of_loggers() != 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        ssln::default_logger = nullptr;
    }

    void CreateLogger(ssln::Verbose verbosity, const std::string& name) {
        logger = ssln::SetupConsole(quill::LogLevel::Debug, verbosity, name);
        ssln::set_default_logger(logger);
    }

    void CheckBasicFormat(const std::string& pattern, bool expect_brackets) {
        // Create a test message
        SSLN_INFO("Test message");
        
        // Check pattern format
        if (!expect_brackets) {
            EXPECT_EQ(pattern.find('['), std::string::npos) 
                << "Format should not contain brackets: " << pattern;
        } else {
            EXPECT_NE(pattern.find('['), std::string::npos) 
                << "Format should contain brackets: " << pattern;
        }
    }

    quill::Logger* logger;
};

TEST_F(VerbosityTest, LiteVerbosity) {
    CreateLogger(ssln::Verbose::kLite, "lite_logger");
    std::string pattern = ssln::detail::GetPattern(ssln::Verbose::kLite);
    CheckBasicFormat(pattern, false);
}

TEST_F(VerbosityTest, LowVerbosity) {
    CreateLogger(ssln::Verbose::kLow, "low_logger");
    std::string pattern = ssln::detail::GetPattern(ssln::Verbose::kLow);
    CheckBasicFormat(pattern, true);
    EXPECT_TRUE(std::regex_search(pattern, std::regex("%\\(time\\)")))
        << "Low format missing time: " << pattern;
}

TEST_F(VerbosityTest, MediumVerbosity) {
    CreateLogger(ssln::Verbose::kMedium, "medium_logger");
    std::string pattern = ssln::detail::GetPattern(ssln::Verbose::kMedium);
    CheckBasicFormat(pattern, true);
    EXPECT_TRUE(std::regex_search(pattern, std::regex("%\\(log_level\\)")))
        << "Medium format missing level: " << pattern;
}

TEST_F(VerbosityTest, HighVerbosity) {
    CreateLogger(ssln::Verbose::kHigh, "high_logger");
    std::string pattern = ssln::detail::GetPattern(ssln::Verbose::kHigh);
    CheckBasicFormat(pattern, true);
    EXPECT_TRUE(std::regex_search(pattern, std::regex("%\\(thread_id\\)")))
        << "High format missing thread id: " << pattern;
}

TEST_F(VerbosityTest, FullVerbosity) {
    CreateLogger(ssln::Verbose::kFull, "full_logger");
    std::string pattern = ssln::detail::GetPattern(ssln::Verbose::kFull);
    CheckBasicFormat(pattern, true);
    EXPECT_TRUE(std::regex_search(pattern, std::regex("%\\(caller_function\\)")))
        << "Full format missing function name: " << pattern;
}

TEST_F(VerbosityTest, UltraVerbosity) {
    CreateLogger(ssln::Verbose::kUltra, "ultra_logger");
    std::string pattern = ssln::detail::GetPattern(ssln::Verbose::kUltra);
    CheckBasicFormat(pattern, true);
    EXPECT_TRUE(std::regex_search(pattern, std::regex("%\\(time\\)")))
        << "Ultra format missing high precision time: " << pattern;
}

int main(int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
