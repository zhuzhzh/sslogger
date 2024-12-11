#include "ssln/sslogger.h"
#include "spdlog/fmt/bin_to_hex.h"
#include <vector>
#include <thread>

int main() {
    // 初始化控制台日志
    ssln::init_console(spdlog::level::debug, ssln::Verbose::kFull, "console");

    // 基本日志测试
    int i = 999;
    spdlog::debug("Debug message {}", i);
    spdlog::info("Important message");

    // 切换到不同的verbosity
    const auto logger = spdlog::get("console");

    // 数组打印测试
    uint8_t data[] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
    // 直接使用spdlog::to_hex
    spdlog::info("Binary data: {}", spdlog::to_hex(data, data + sizeof(data)));

    // 使用vector
    std::vector<uint8_t> vec_data = {
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88
    };
    spdlog::info("Vector data: {}", spdlog::to_hex(vec_data));

    // 部分数据
    spdlog::debug("First 4 bytes: {}", 
        spdlog::to_hex(vec_data.begin(), vec_data.begin() + 4));

    // 不同格式的十六进制输出
    uint8_t large_data[] = {
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
        0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xFF, 0x12, 0x34,
        0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x01, 0x23,
        0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xFF, 0x12, 0x34,
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88
    };

    // 使用源文件信息
    SPDLOG_INFO("Large data with source info: {}", 
        spdlog::to_hex(large_data, large_data + sizeof(large_data)));

    // 切换到文件日志
    ssln::init_async_file("logs", "advanced.log", 
        spdlog::level::debug, ssln::Verbose::kFull, "file_logger");
    
    auto file_logger = spdlog::get("file_logger");
    spdlog::set_default_logger(file_logger);

    // 在文件中记录十六进制数据
    SPDLOG_DEBUG("Hex data in file: {}", 
        spdlog::to_hex(vec_data.begin(), vec_data.end()));

    // 编译期日志测试
    SPDLOG_DEBUG("Debug message at compile time");
    SPDLOG_INFO("Info message at compile time");

    // 使用不同的verbosity级别
    std::vector<uint8_t> small_data = {0x12, 0x34, 0x56, 0x78};
    
    // Lite format
    ssln::init_console(spdlog::level::debug, ssln::Verbose::kLite, "console2");
    auto lite_logger = spdlog::get("console2");
    spdlog::set_default_logger(lite_logger);
    spdlog::info("Small data (lite): {}", spdlog::to_hex(small_data));

    // Full format
    ssln::init_console(spdlog::level::debug, ssln::Verbose::kFull, "console3");
    auto full_logger = spdlog::get("console3");
    spdlog::set_default_logger(full_logger);
    SPDLOG_INFO("Small data (full): {}", spdlog::to_hex(small_data));

    return 0;
}