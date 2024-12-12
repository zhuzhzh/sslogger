#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
// tests/test_hex_logging.cc
#include "ssln/sslogger.h"
#include <gtest/gtest.h>
#include <sstream>
#include <regex>
#include <spdlog/sinks/ostream_sink.h>

class HexLoggingTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_stream_ = std::make_shared<std::ostringstream>();
        auto ostream_sink = std::make_shared<spdlog::sinks::ostream_sink_st>(*test_stream_);
        auto logger = std::make_shared<spdlog::logger>("hex_logger", ostream_sink);
        
        // Ensure both runtime and compile-time levels allow debug messages
        EXPECT_EQ(SPDLOG_ACTIVE_LEVEL, SPDLOG_LEVEL_DEBUG) << "Compile-time log level should be DEBUG";
        logger->set_level(spdlog::level::debug);
        EXPECT_EQ(logger->level(), spdlog::level::debug) << "Runtime log level should be DEBUG";
        
        logger->set_pattern(ssln::detail::GetPattern(ssln::Verbose::kMedium));
        spdlog::set_default_logger(logger);
    }

    void TearDown() override {
        spdlog::drop_all();
    }

    // Helper function to check if string contains expected hex pattern
    bool ContainsHexPattern(const std::string& output, const std::string& expected_hex) {
        // Remove all spaces for easier comparison.
        std::string clean_expected = expected_hex;
        clean_expected.erase(
            std::remove(clean_expected.begin(), clean_expected.end(), ' '),
            clean_expected.end());
        
        // Build regex pattern to match possible hex output formats.
        std::string pattern = "[\\s\\S]*?";  // Any character including newlines.
        for (size_t i = 0; i < clean_expected.length(); i += 2) {
            if (i > 0) {
                pattern += "[\\s:]*";  // Allow spaces and colons.
            }
            pattern += clean_expected.substr(i, 2);
        }
        
        std::regex hex_pattern(pattern, std::regex::icase);
        bool found = std::regex_search(output, hex_pattern);
        
        if (!found) {
            std::cout << "Expected hex pattern not found.\n"
                      << "Expected: " << expected_hex << "\n"
                      << "Clean expected: " << clean_expected << "\n"
                      << "Pattern: " << pattern << "\n"
                      << "Actual output: " << output << std::endl;
        }
        return found;
    }

    std::shared_ptr<std::ostringstream> test_stream_;
};

TEST_F(HexLoggingTest, ArrayHexLogging) {
    uint8_t data[] = {0x12, 0x34, 0x56, 0x78};
    SPDLOG_INFO("Array data: {}", spdlog::to_hex(data, data + sizeof(data)));
    
    std::string output = test_stream_->str();
    EXPECT_FALSE(output.empty()) << "Log output should not be empty";
    EXPECT_TRUE(ContainsHexPattern(output, "12 34 56 78"));
}

TEST_F(HexLoggingTest, VectorHexLogging) {
    std::vector<uint8_t> vec_data = {0x9A, 0xBC, 0xDE, 0xF0};
    SPDLOG_DEBUG("Vector data: {}", spdlog::to_hex(vec_data));
    
    std::string output = test_stream_->str();
    EXPECT_FALSE(output.empty()) << "Log output should not be empty";
    EXPECT_TRUE(ContainsHexPattern(output, "9a bc de f0"));
}

TEST_F(HexLoggingTest, EmptyVectorHexLogging) {
    std::vector<uint8_t> empty_vec;
    SPDLOG_DEBUG("Empty vector: {}", spdlog::to_hex(empty_vec));
    
    std::string output = test_stream_->str();
    EXPECT_FALSE(output.empty()) << "Log output should not be empty";
    EXPECT_TRUE(output.find("Empty vector") != std::string::npos);
}

TEST_F(HexLoggingTest, LargeVectorHexLogging) {
    std::vector<uint8_t> large_vec;
    for (int i = 0; i < 16; ++i) {
        large_vec.push_back(static_cast<uint8_t>(i));
    }
    SPDLOG_DEBUG("Large vector: {}", spdlog::to_hex(large_vec));
    
    std::string output = test_stream_->str();
    EXPECT_FALSE(output.empty()) << "Log output should not be empty";
    EXPECT_TRUE(ContainsHexPattern(output, "00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f"));
}

int main(int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}