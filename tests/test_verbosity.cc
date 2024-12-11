// tests/test_verbosity.cc
#include "ssln/sslogger.h"
#include <gtest/gtest.h>

TEST(LoggerTest, DifferentVerbosityLevels) {
    // Lite verbosity
    ssln::init_console("lite_logger", spdlog::level::debug, ssln::Verbose::kLite);
    auto logger = spdlog::get("lite_logger");
    spdlog::set_default_logger(logger);
    SPDLOG_INFO("Lite format message");
    
    // Medium verbosity
    ssln::init_console("medium_logger", spdlog::level::debug, ssln::Verbose::kMedium);
    logger = spdlog::get("medium_logger");
    spdlog::set_default_logger(logger);
    SPDLOG_INFO("Medium format message");
    
    // Full verbosity
    ssln::init_console("full_logger", spdlog::level::debug, ssln::Verbose::kFull);
    logger = spdlog::get("full_logger");
    spdlog::set_default_logger(logger);
    SPDLOG_INFO("Full format message");
}

int main(int argc, char const *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}