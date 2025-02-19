# SSLogger

SSLogger 是一个基于 quill 的 C++ 日志库封装，提供了简单易用且高性能的日志功能。

## 主要特性

- 支持同步和异步日志模式
- 支持控制台和文件输出
- 提供多种日志格式预设（从简单到详细）
- 支持十六进制数据输出
- 支持性能计时功能
- 支持环境变量配置
- 线程安全
- 高性能（异步模式下可达百万级/秒）

## 使用方法

### 基本用法

```cpp
#include "ssln/sslogger.h"

int main() {
    // 初始化控制台日志，指定 logger name
    ssln::SetupConsole(quill::LogLevel::Debug, ssln::Verbose::kMedium, "console_logger");
    
    // 使用日志宏
    SSLN_INFO("Hello, SSLogger!");
    SSLN_DEBUG("Debug message: {}", 42);

    // 获取指定名称的 logger
    auto logger = ssln::get_logger("console_logger");
    SSLN_LOG_INFO(logger, "Using logger-specific macro");
}
```

### 文件日志

```cpp
// 普通文件日志
ssln::SetupFile("app.log", quill::LogLevel::Info, ssln::Verbose::kMedium, "file_logger");

// 滚动文件日志（自动分割大文件）
ssln::SetupRotatingFile(
    "app.log",
    1024*1024*10,  // 最大文件大小
    5,             // 最大文件数
    quill::LogLevel::Info,
    ssln::Verbose::kMedium,
    "rotating_logger"
);

// 性能日志（最简格式）
ssln::SetupPerfFile("perf.log", quill::LogLevel::Info, ssln::Verbose::kLite, "perf_logger");
```

### 日志格式预设

```cpp
// 最简格式：仅消息
ssln::SetupConsole(quill::LogLevel::Info, ssln::Verbose::kLite);
SSLN_INFO("Message only");  // 输出: Message only

// 低详细度：时间 + 消息
ssln::SetupConsole(quill::LogLevel::Info, ssln::Verbose::kLow);
SSLN_INFO("With time");  // 输出: [23:59:59.123] With time

// 中等详细度：时间 + 级别 + 位置
ssln::SetupConsole(quill::LogLevel::Info, ssln::Verbose::kMedium);
SSLN_INFO("Standard");  // 输出: [23:59:59.123][INFO][file.cpp:42] Standard

// 高详细度：时间 + 级别 + 线程 + 位置
ssln::SetupConsole(quill::LogLevel::Info, ssln::Verbose::kHigh);

// 完整格式：完整时间 + 级别 + 线程 + 函数 + 行号
ssln::SetupConsole(quill::LogLevel::Info, ssln::Verbose::kFull);

// 超详细格式：毫秒精度时间戳 + 所有信息
ssln::SetupConsole(quill::LogLevel::Info, ssln::Verbose::kUltra);
```

### 十六进制数据输出

```cpp
// 输出数组数据
uint8_t data[] = {0x12, 0x34, 0x56, 0x78};
SSLN_INFO("Array data: {}", quill::utility::to_hex(data, sizeof(data)));

// 输出 vector 数据
std::vector<uint8_t> vec_data = {0x9A, 0xBC, 0xDE, 0xF0};
SSLN_INFO("Vector data: {}", quill::utility::to_hex(vec_data.data(), vec_data.size()));
```

### 性能计时

```cpp
quill::StopWatchTsc sw;  // 使用 TSC 计时器，更高精度
// ... 执行一些操作 ...
SSLN_INFO("Operation took {} seconds", sw);

// 支持不同时间单位
auto ns = sw.elapsed_as<std::chrono::nanoseconds>();
auto ms = sw.elapsed_as<std::chrono::milliseconds>();
```

## 环境变量配置

SSLogger 支持通过环境变量动态配置日志行为，无需修改代码：

### 日志级别
- `SSLN_LOG_LEVEL`: 设置全局日志级别
  - 可选值: "trace", "debug", "info", "warn", "error", "critical", "off"
  - 示例: `export SSLN_LOG_LEVEL=debug`

### 输出格式
- `SSLN_VERBOSITY`: 设置输出格式详细程度
  - 可选值: "lite", "low", "medium", "high", "full", "ultra"
  - 示例: `export SSLN_VERBOSITY=full`

### 文件输出
- `SSLN_LOG_DIR`: 设置日志文件目录
  - 示例: `export SSLN_LOG_DIR=/var/log/myapp`
- `SSLN_LOG_NAME`: 设置日志文件名
  - 示例: `export SSLN_LOG_NAME=app.log`

### 文件轮转
- `SSLN_MAX_FILE_SIZE`: 设置单个日志文件最大大小（字节）
  - 示例: `export SSLN_MAX_FILE_SIZE=10485760`  # 10MB
- `SSLN_MAX_FILES`: 设置最大保留文件数
  - 示例: `export SSLN_MAX_FILES=5`

### 优先级
环境变量配置会覆盖代码中的默认设置。例如：
```cpp
// 代码中设置 Info 级别
auto logger = ssln::SetupConsole(quill::LogLevel::Info);

// 如果设置了环境变量 SSLN_LOG_LEVEL=debug
// 实际日志级别将是 Debug
```

## 编译要求

- C++17 或更高版本
- CMake 3.10 或更高版本
- quill 库

## 注意事项

- 异步模式在程序异常退出时可能丢失最后一些消息
- 建议在发布版本中使用 `QUILL_ACTIVE_LEVEL` 定义适当的日志级别
- 文件日志需要确保目录存在且有写入权限
- logger name 在全局必须唯一，重复的 name 会返回已存在的 logger
- 环境变量的修改会影响所有新创建的 logger，但不会影响已存在的 logger

## 许可证

MIT License
