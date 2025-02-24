// tests/test_verbosity.cc
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

class VerbosityTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize backend
    }

    void TearDown() override {
    }

    void CreateLogger(ssln::Verbose verbosity, const std::string& name) {
        logger = ssln::SetupConsoleLogger(name, verbosity, quill::LogLevel::Debug);
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
    std::string pattern = ssln::GetPattern(ssln::Verbose::kLite);
    CheckBasicFormat(pattern, false);
}

TEST_F(VerbosityTest, LowVerbosity) {
    CreateLogger(ssln::Verbose::kLow, "low_logger");
    std::string pattern = ssln::GetPattern(ssln::Verbose::kLow);
    CheckBasicFormat(pattern, true);
    EXPECT_TRUE(std::regex_search(pattern, std::regex("%\\(time\\)")))
        << "Low format missing time: " << pattern;
}

TEST_F(VerbosityTest, MediumVerbosity) {
    CreateLogger(ssln::Verbose::kMedium, "medium_logger");
    std::string pattern = ssln::GetPattern(ssln::Verbose::kMedium);
    CheckBasicFormat(pattern, true);
    EXPECT_TRUE(std::regex_search(pattern, std::regex("%\\(log_level\\)")))
        << "Medium format missing level: " << pattern;
}

TEST_F(VerbosityTest, HighVerbosity) {
    CreateLogger(ssln::Verbose::kHigh, "high_logger");
    std::string pattern = ssln::GetPattern(ssln::Verbose::kHigh);
    CheckBasicFormat(pattern, true);
    EXPECT_TRUE(std::regex_search(pattern, std::regex("%\\(thread_id\\)")))
        << "High format missing thread id: " << pattern;
}

TEST_F(VerbosityTest, FullVerbosity) {
    CreateLogger(ssln::Verbose::kFull, "full_logger");
    std::string pattern = ssln::GetPattern(ssln::Verbose::kFull);
    CheckBasicFormat(pattern, true);
    EXPECT_TRUE(std::regex_search(pattern, std::regex("%\\(caller_function\\)")))
        << "Full format missing function name: " << pattern;
}

TEST_F(VerbosityTest, UltraVerbosity) {
    CreateLogger(ssln::Verbose::kUltra, "ultra_logger");
    std::string pattern = ssln::GetPattern(ssln::Verbose::kUltra);
    CheckBasicFormat(pattern, true);
    EXPECT_TRUE(std::regex_search(pattern, std::regex("%\\(time\\)")))
        << "Ultra format missing high precision time: " << pattern;
}

int main(int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
