#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
// tests/test_verbosity.cc
#include "ssln/sslogger.h"
#include <gtest/gtest.h>
#include <sstream>
#include <regex>
#include <spdlog/sinks/ostream_sink.h>

class VerbosityTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_stream_ = std::make_shared<std::ostringstream>();
        sink_ = std::make_shared<spdlog::sinks::ostream_sink_st>(*test_stream_);
    }

    void TearDown() override {
        spdlog::drop_all();
    }

    void create_logger(ssln::Verbose verbosity, const std::string& name) {
        auto logger = std::make_shared<spdlog::logger>(name, sink_);
        logger->set_level(spdlog::level::debug);
        logger->set_pattern(ssln::detail::get_pattern(verbosity));
        spdlog::set_default_logger(logger);
        test_stream_->str("");
    }

    void check_basic_format(const std::string& output, bool expect_brackets) {
        EXPECT_FALSE(output.empty()) << "Output should not be empty";
        EXPECT_TRUE(output.find("Test message") != std::string::npos) 
            << "Output missing test message: " << output;
        
        if (!expect_brackets) {
            EXPECT_EQ(output.find('['), std::string::npos) 
                << "Format should not contain brackets: " << output;
        } else {
            EXPECT_NE(output.find('['), std::string::npos) 
                << "Format should contain brackets: " << output;
        }
    }

    std::shared_ptr<std::ostringstream> test_stream_;
    std::shared_ptr<spdlog::sinks::ostream_sink_st> sink_;
};

TEST_F(VerbosityTest, LiteVerbosity) {
    create_logger(ssln::Verbose::kLite, "lite_logger");
    SPDLOG_INFO("Test message");
    check_basic_format(test_stream_->str(), false);
}

TEST_F(VerbosityTest, LowVerbosity) {
    create_logger(ssln::Verbose::kLow, "low_logger");
    SPDLOG_INFO("Test message");
    std::string output = test_stream_->str();
    check_basic_format(output, true);
    EXPECT_TRUE(std::regex_search(output, std::regex(R"(\[\d{2}:\d{2}:\d{2})")))
        << "Low format missing time: " << output;
}

TEST_F(VerbosityTest, MediumVerbosity) {
    create_logger(ssln::Verbose::kMedium, "medium_logger");
    SPDLOG_INFO("Test message");
    std::string output = test_stream_->str();
    check_basic_format(output, true);
    EXPECT_TRUE(std::regex_search(output, std::regex(R"(\[I\])")))
        << "Medium format missing level: " << output;
}

TEST_F(VerbosityTest, HighVerbosity) {
    create_logger(ssln::Verbose::kHigh, "high_logger");
    SPDLOG_INFO("Test message");
    std::string output = test_stream_->str();
    check_basic_format(output, true);
    EXPECT_TRUE(std::regex_search(output, std::regex(R"(\[thread \d+\])")))
        << "High format missing thread id: " << output;
}

TEST_F(VerbosityTest, FullVerbosity) {
    create_logger(ssln::Verbose::kFull, "full_logger");
    SPDLOG_INFO("Test message");
    std::string output = test_stream_->str();
    check_basic_format(output, true);
    EXPECT_TRUE(std::regex_search(output, std::regex(R"(\[\d{4}-\d{2}-\d{2})")))
        << "Full format missing date: " << output;
}

TEST_F(VerbosityTest, UltraVerbosity) {
    create_logger(ssln::Verbose::kUltra, "ultra_logger");
    SPDLOG_INFO("Test message");
    std::string output = test_stream_->str();
    check_basic_format(output, true);
    EXPECT_TRUE(std::regex_search(output, std::regex(R"(\.\d{9}\])")))
        << "Ultra format missing microseconds: " << output;
}

int main(int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}