// tests/test_hex_logging.cc
#include "ssln/sslogger.h"
#include <gtest/gtest.h>

TEST(LoggerTest, HexLogging) {
    ssln::init_console("hex_logger", spdlog::level::debug, ssln::Verbose::kMedium);
    
    // 测试数组输出
    uint8_t data[] = {0x12, 0x34, 0x56, 0x78};
    SPDLOG_INFO("Array data: {}", spdlog::to_hex(data, data + sizeof(data)));
    
    // 测试vector输出
    std::vector<uint8_t> vec_data = {0x9A, 0xBC, 0xDE, 0xF0};
    SPDLOG_DEBUG("Vector data: {}", spdlog::to_hex(vec_data));
}

int main(int argc, char const *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}