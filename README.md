# SSLogger

SSLogger is a C++ logging library wrapper based on quill, providing easy-to-use and high-performance logging functionality.

## Key Features

- Support for synchronous and asynchronous logging modes
- Console and file output support
- Multiple logging format presets (from simple to detailed)
- Hexadecimal data output support
- Performance timing support
- Environment variable configuration
- Thread safety
- High performance (millions of messages per second in async mode)

## Usage

### Basic Usage

```cpp
#include "ssln/sslogger.h"

int main() {
    // Initialize console logger with a specified name
    ssln::SetupConsole(quill::LogLevel::Debug, ssln::Verbose::kMedium, "console_logger");
    
    // Using logging macros
    SSLN_INFO("Hello, SSLogger!");
    SSLN_DEBUG("Debug message: {}", 42);

    // Get logger by name
    auto logger = ssln::get_logger("console_logger");
    SSLN_LOG_INFO(logger, "Using logger-specific macro");
}
```

### File Logging

```cpp
// Standard file logger
ssln::SetupFile("app.log", quill::LogLevel::Info, ssln::Verbose::kMedium, "file_logger");

// Rotating file logger (automatic file splitting)
ssln::SetupRotatingFile(
    "app.log",
    1024*1024*10,  // Maximum file size
    5,             // Maximum number of files
    quill::LogLevel::Info,
    ssln::Verbose::kMedium,
    "rotating_logger"
);

// Performance logger (minimal format)
ssln::SetupPerfFile("perf.log", quill::LogLevel::Info, ssln::Verbose::kLite, "perf_logger");
```

### Logging Format Presets

```cpp
// Lite format: message only
ssln::SetupConsole(quill::LogLevel::Info, ssln::Verbose::kLite);
SSLN_INFO("Message only");  // Output: Message only

// Low verbosity: time + message
ssln::SetupConsole(quill::LogLevel::Info, ssln::Verbose::kLow);
SSLN_INFO("With time");  // Output: [23:59:59.123] With time

// Medium verbosity: time + level + location
ssln::SetupConsole(quill::LogLevel::Info, ssln::Verbose::kMedium);
SSLN_INFO("Standard");  // Output: [23:59:59.123][INFO][file.cpp:42] Standard

// High verbosity: time + level + thread + location
ssln::SetupConsole(quill::LogLevel::Info, ssln::Verbose::kHigh);

// Full format: full time + level + thread + function + line
ssln::SetupConsole(quill::LogLevel::Info, ssln::Verbose::kFull);

// Ultra format: millisecond precision timestamp + all details
ssln::SetupConsole(quill::LogLevel::Info, ssln::Verbose::kUltra);
```

### Hexadecimal Data Output

```cpp
// Output array data
uint8_t data[] = {0x12, 0x34, 0x56, 0x78};
SSLN_INFO("Array data: {}", quill::utility::to_hex(data, sizeof(data)));

// Output vector data
std::vector<uint8_t> vec_data = {0x9A, 0xBC, 0xDE, 0xF0};
SSLN_INFO("Vector data: {}", quill::utility::to_hex(vec_data.data(), vec_data.size()));
```

### Performance Timing

```cpp
quill::StopWatchTsc sw;  // Using TSC timer for higher precision
// ... perform operations ...
SSLN_INFO("Operation took {} seconds", sw);

// Support for different time units
auto ns = sw.elapsed_as<std::chrono::nanoseconds>();
auto ms = sw.elapsed_as<std::chrono::milliseconds>();
```

## Environment Variable Configuration

SSLogger supports dynamic configuration through environment variables without code changes:

### Log Level
- `SSLN_LOG_LEVEL`: Set global log level
  - Values: "trace", "debug", "info", "warn", "error", "critical", "off"
  - Example: `export SSLN_LOG_LEVEL=debug`

### Output Format
- `SSLN_VERBOSITY`: Set output format verbosity
  - Values: "lite", "low", "medium", "high", "full", "ultra"
  - Example: `export SSLN_VERBOSITY=full`

### File Output
- `SSLN_LOG_DIR`: Set log file directory
  - Example: `export SSLN_LOG_DIR=/var/log/myapp`
- `SSLN_LOG_NAME`: Set log file name
  - Example: `export SSLN_LOG_NAME=app.log`

### File Rotation
- `SSLN_MAX_FILE_SIZE`: Set maximum size for single log file (bytes)
  - Example: `export SSLN_MAX_FILE_SIZE=10485760`  # 10MB
- `SSLN_MAX_FILES`: Set maximum number of backup files
  - Example: `export SSLN_MAX_FILES=5`

### Priority
Environment variable settings override code defaults. For example:
```cpp
// Set Info level in code
auto logger = ssln::SetupConsole(quill::LogLevel::Info);

// If SSLN_LOG_LEVEL=debug is set
// Actual log level will be Debug
```

## Build Requirements

- C++17 or higher
- CMake 3.10 or higher
- quill library

## Important Notes

- Asynchronous mode may lose last messages on abnormal program termination
- Recommended to define appropriate `QUILL_ACTIVE_LEVEL` in release builds
- File logging requires existing directory with write permissions
- Logger names must be unique globally; duplicate names return existing logger
- Environment variable changes affect newly created loggers but not existing ones

## License

MIT License
