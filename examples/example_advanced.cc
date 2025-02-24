#include "ssln/sslogger.h"
#include "./tlm_payload_format.h"
#include "quill/std/Vector.h"
#include "quill/std/Pair.h"
#include "quill/std/Array.h"
#include "quill/Utility.h"
#include <vector>
#include <thread>

int main() {
    // Initialize console logger
    ssln::set_default_logger(ssln::console_logger);

    // Basic logging test
    int i = 999;
    SSLN_DEBUG("Debug message {}", i);
    SSLN_INFO("Important message");

    // Array data test
    uint8_t data[] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
    // Using quill's hex formatting
    SSLN_INFO("Binary data: {}", quill::utility::to_hex(data, sizeof(data)));

    // Using vector
    std::vector<uint8_t> vec_data = {
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88
    };
    SSLN_INFO("Vector data: {}", quill::utility::to_hex(vec_data.data(), vec_data.size()));
    SSLN_INFO("Vector data: {}", vec_data);
    SSLN_INFO("Vector data: {::#06x}", vec_data);

    // Partial data
    auto first_four = std::vector<uint8_t>(vec_data.begin(), vec_data.begin() + 4);
    SSLN_DEBUG("First 4 bytes: {}", quill::utility::to_hex(first_four.data(), first_four.size()));

    // Test TlmPayload logging
    std::vector<uint8_t> payload_data = {
        // Data section
        0x12, 0x34, 0x56, 0x78,
        // Byte enable section
        0xFF, 0xFF,
        // AXUser section
        0xAA,
        // XUser section
        0xBB
    };

    ssln::hybrid::TlmPayload payload{
        .id = 0x123,
        .command = 1,  // write
        .address = 0x1000,
        .data_length = 4,
        .byte_enable_length = 2,
        .axuser_length = 1,
        .xuser_length = 1,
        .streaming_width = 4,
        .response = 0,  // okay
        .data = payload_data.data()
    };

    SSLN_INFO("Logging TLM payload: {}", payload);

    // Different hex output formats
    uint8_t large_data[] = {
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
        0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xFF, 0x12, 0x34,
        0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x01, 0x23,
        0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xFF, 0x12, 0x34,
        0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88
    };

    // Using source file info
    SSLN_INFO("Large data with source info: {}", quill::utility::to_hex(large_data, sizeof(large_data)));

    // Switch to file logger
    ssln::set_default_logger(ssln::hybrid_logger);

    // Log hex data to file
    SSLN_DEBUG("Hex data in file: {:#x}", vec_data);

    // Compile-time log level test
    SSLN_DEBUG("Debug message at compile time");
    SSLN_INFO("Info message at compile time");
    SSLN_LOG_INFO(ssln::hybrid_logger, "this is the full message with source information");

    // Using different verbosity levels
    std::vector<uint8_t> small_data = {0x12, 0x34, 0x56, 0x78};
    
    ssln::set_default_logger(ssln::console_logger);
    SSLN_INFO("Small data (lite): {}", quill::utility::to_hex(small_data.data(), small_data.size()));


    return 0;
}
