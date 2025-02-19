#define QUILL_COMPILE_ACTIVE_LOG_LEVEL QUILL_COMPILE_ACTIVE_LOG_LEVEL_TRACE_L3
// tests/test_hex_logging.cc
#include "ssln/sslogger.h"
#include "ssln/sslogger_macros.h"

// C++ standard library headers
#include <fstream>
#include <sstream>
#include <regex>
#include <thread>
#include <chrono>

// Google Test headers
#include <gtest/gtest.h>

// Quill headers
#include "quill/Frontend.h"
#include "quill/Utility.h"

class HexLoggingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize console logger with minimal pattern for testing
        logger = ssln::SetupConsole(quill::LogLevel::Debug, ssln::Verbose::kLite);
        ASSERT_TRUE(logger != nullptr) << "Failed to create logger";
        ssln::set_default_logger(logger);
    }

    void TearDown() override {
        if (logger) {
            logger->flush_log();
        
        size_t const total_loggers = quill::Frontend::get_number_of_loggers();
        quill::Frontend::remove_logger(logger);
        while(quill::Frontend::get_number_of_loggers()!=(total_loggers-1)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        ssln::default_logger = nullptr;
        logger = nullptr;
        }
    }

    // Helper function to check if string contains expected hex pattern
    bool ContainsHexPattern(const std::string& hex_str, const std::string& expected_hex) {
        // Remove all spaces for easier comparison
        std::string clean_hex = hex_str;
        clean_hex.erase(
            std::remove_if(clean_hex.begin(), clean_hex.end(), 
                          [](char c) { return std::isspace(c); }),
            clean_hex.end());
        
        std::string clean_expected = expected_hex;
        clean_expected.erase(
            std::remove_if(clean_expected.begin(), clean_expected.end(), 
                          [](char c) { return std::isspace(c); }),
            clean_expected.end());
        
        return clean_hex.find(clean_expected) != std::string::npos;
    }

    quill::Logger* logger;
};

TEST_F(HexLoggingTest, ArrayHexLogging) {
    uint8_t data[] = {0x12, 0x34, 0x56, 0x78};
    std::string hex_str = quill::utility::to_hex(data, sizeof(data));
    EXPECT_TRUE(ContainsHexPattern(hex_str, "12 34 56 78"));
    SSLN_LOG_INFO(logger,"Hex data: {}", hex_str);
}

TEST_F(HexLoggingTest, VectorHexLogging) {
    std::vector<uint8_t> vec_data = {0x9A, 0xBC, 0xDE, 0xF0};
    std::string hex_str = quill::utility::to_hex(vec_data.data(), vec_data.size());
    EXPECT_TRUE(ContainsHexPattern(hex_str, "9A BC DE F0"));
    SSLN_LOG_INFO(logger,"Hex data: {}", hex_str);
}

/*
TEST_F(HexLoggingTest, EmptyVectorHexLogging) {
    std::vector<uint8_t> empty_vec;
    std::string hex_str = quill::utility::to_hex(empty_vec.data(), empty_vec.size());
    EXPECT_TRUE(hex_str.empty());
}
*/

TEST_F(HexLoggingTest, LargeVectorHexLogging) {
    std::vector<uint8_t> large_vec;
    for (int i = 0; i < 16; ++i) {
        large_vec.push_back(static_cast<uint8_t>(i));
    }
    std::string hex_str = quill::utility::to_hex(large_vec.data(), large_vec.size());
    EXPECT_TRUE(ContainsHexPattern(hex_str, 
        "00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F"));
    SSLN_LOG_INFO(logger,"Hex data: {}", hex_str);
    
}

TEST_F(HexLoggingTest, LoggingWithFormat) {
    uint8_t data[] = {0x12, 0x34, 0x56, 0x78};
    SSLN_LOG_INFO(logger,"Hex data: {}", quill::utility::to_hex(data, sizeof(data)));
}

int main(int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
