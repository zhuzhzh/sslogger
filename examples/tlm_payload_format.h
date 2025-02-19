 #ifndef SSLN_TLM_PAYLOAD_FORMAT_H_
#define SSLN_TLM_PAYLOAD_FORMAT_H_

#include "quill/DirectFormatCodec.h"
#include "quill/Utility.h"
#include <cstdint>

namespace ssln {
namespace hybrid {

// TLM payload structure for IPC communication
struct TlmPayload {
    uint64_t id;
    uint8_t command;              // Command type
    uint64_t address;            // Target address
    uint32_t data_length;        // Length of data
    uint32_t byte_enable_length; // Length of byte enable
    uint32_t axuser_length;      // Length of axuser
    uint32_t xuser_length;       // Length of xuser
    uint32_t streaming_width;    // Streaming width
    uint8_t response;           // Response status
    uint8_t* data;                // Variable length data followed by byte enable
};

} // namespace hybrid
} // namespace ssln

// Formatter for TlmPayload
template <>
struct fmtquill::formatter<ssln::hybrid::TlmPayload> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    auto format(ssln::hybrid::TlmPayload const& payload, format_context& ctx) const {
        // Get pointers to different sections of data
        const uint8_t* data_ptr = payload.data;
        const uint8_t* byte_enable_ptr = data_ptr + payload.data_length;
        const uint8_t* axuser_ptr = byte_enable_ptr + payload.byte_enable_length;
        const uint8_t* xuser_ptr = axuser_ptr + payload.axuser_length;

        // Convert command and response to strings
        const char* cmd_str = payload.command == 1 ? "write" : 
                            payload.command == 0 ? "read" : "other";
        const char* resp_str = payload.response == 0 ? "okay" : "error";

        // Format basic fields
        auto out = fmtquill::format_to(ctx.out(), 
            "TLM[id={}, cmd={}, addr={:#x}, sw={}, resp={}, ",
            payload.id, 
            cmd_str,
            payload.address,
            payload.streaming_width,
            resp_str);

        // Format data section
        out = fmtquill::format_to(out, "data=");
        out = fmtquill::format_to(out, "{}", 
            quill::utility::to_hex(data_ptr, payload.data_length));

        // Format byte enable section
        out = fmtquill::format_to(out, ", be=");
        out = fmtquill::format_to(out, "{}", 
            quill::utility::to_hex(byte_enable_ptr, payload.byte_enable_length));

        // Format axuser section
        out = fmtquill::format_to(out, ", axuser=");
        out = fmtquill::format_to(out, "{}", 
            quill::utility::to_hex(axuser_ptr, payload.axuser_length));

        // Format xuser section
        out = fmtquill::format_to(out, ", xuser=");
        out = fmtquill::format_to(out, "{}", 
            quill::utility::to_hex(xuser_ptr, payload.xuser_length));

        return fmtquill::format_to(out, "]");
    }
};

// Codec for TlmPayload
template <>
struct quill::Codec<ssln::hybrid::TlmPayload> : quill::DirectFormatCodec<ssln::hybrid::TlmPayload> {};

#endif  // SSLN_TLM_PAYLOAD_FORMAT_H_