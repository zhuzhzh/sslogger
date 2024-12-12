# SSLogger

SSLogger 是一个基于 spdlog 的 C++ 日志库封装，提供了简单易用且高性能的日志功能。

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
    ssln::init_console(spdlog::level::debug, ssln::Verbose::kMedium, "console_logger");
    
    // 方式1：使用全局宏
    SPDLOG_INFO("Hello, SSLogger!");
    SPDLOG_DEBUG("Debug message: {}", 42);

    // 方式2：获取指定名称的 logger
    auto logger = spdlog::get("console_logger");
    logger->info("Using logger pointer directly");
    SPDLOG_LOGGER_INFO(logger, "Using logger-specific macro");
}
```

### 文件日志

```cpp
// 同步文件日志
ssln::init_sync_file("logs", "app.log", spdlog::level::info, ssln::Verbose::kMedium, "sync_logger");
auto sync_logger = spdlog::get("sync_logger");

// 异步文件日志
ssln::init_async_file("logs", "app.log", spdlog::level::info, ssln::Verbose::kMedium, "async_logger");
auto async_logger = spdlog::get("async_logger");

// 滚动文件日志（自动分割大文件）
ssln::init_rotating_file(
    "logs", "app.log",
    1024*1024*10, 5,  // 最大文件大小和文件数
    spdlog::level::info,
    ssln::Verbose::kMedium,
    "rotating_logger"
);
auto rotating_logger = spdlog::get("rotating_logger");

// 使用获取的 logger
sync_logger->info("Using sync logger");
SPDLOG_LOGGER_INFO(async_logger, "Using async logger");
rotating_logger->warn("Using rotating logger");
```

### 日志格式预设

```cpp
// 最简格式：仅消息
ssln::init_console(spdlog::level::info, ssln::Verbose::kLite, "lite_logger");
auto logger = spdlog::get("lite_logger");
logger->info("Message only");  // 输出: Message only

// 低详细度：时间 + 消息
ssln::init_console(spdlog::level::info, ssln::Verbose::kLow, "low_logger");
SPDLOG_LOGGER_INFO(spdlog::get("low_logger"), "With time");  // 输出: [HH:MM:SS.ms] With time

// 中等详细度：时间 + 级别 + 位置
ssln::init_console(spdlog::level::info, ssln::Verbose::kMedium, "medium_logger");
SPDLOG_INFO("Standard");  // 输出: [HH:MM:SS.ms][INFO][file.cpp:42] Standard

// 高详细度：时间 + 级别 + 线程 + 位置
ssln::init_console(spdlog::level::info, ssln::Verbose::kHigh, "high_logger");

// 完整格式：完整时间 + 级别 + 线程 + 函数 + 行号
ssln::init_console(spdlog::level::info, ssln::Verbose::kFull, "full_logger");

// 超详细格式：毫秒精度时间戳 + 所有信息
ssln::init_console(spdlog::level::info, ssln::Verbose::kUltra, "ultra_logger");
```

### 十六进制数据输出

```cpp
auto logger = spdlog::get("hex_logger");

// 输出数组数据
uint8_t data[] = {0x12, 0x34, 0x56, 0x78};
logger->info("Array data: {}", spdlog::to_hex(data, data + sizeof(data)));

// 输出 vector 数据
std::vector<uint8_t> vec_data = {0x9A, 0xBC, 0xDE, 0xF0};
SPDLOG_LOGGER_INFO(logger, "Vector data: {}", spdlog::to_hex(vec_data));
```

### 性能计时

```cpp
auto logger = spdlog::get("timer_logger");
spdlog::stopwatch sw;

// ... 执行一些操作 ...

// 多种方式记录耗时
logger->info("Operation took {} seconds", sw);
SPDLOG_LOGGER_INFO(logger, "Operation took {} seconds", sw);
```

### 环境变量配置

- `SSLN_LOG_LEVEL`: 设置日志级别 (trace, debug, info, warn, error, critical, off)
- `SSLN_VERBOSITY`: 设置详细程度 (lite, low, medium, high, full, ultra)
- `SSLN_LOG_DIR`: 设置日志文件目录
- `SSLN_LOG_NAME`: 设置日志文件名
- `SSLN_QUEUE_SIZE`: 设置异步模式队列大小（默认 8192）
- `SSLN_THREADS`: 设置异步模式线程数（默认 1）
- `SSLN_FLUSH_SECS`: 设置自动刷新间隔（秒）

## 性能基准

使用提供的 benchmark 工具可以测试不同模式下的性能：

```bash
./benchmarks
```

典型性能数据（百万消息测试）：
- 同步模式：~50,000 消息/秒
- 异步阻塞模式：~500,000 消息/秒
- 异步覆盖模式：~1,700,000 消息/秒

## 编译要求

- C++14 或更高版本
- CMake 3.10 或更高版本
- spdlog 库

## 注意事项

- 异步模式在程序异常退出时可能丢失最后一些消息
- 异步覆盖模式在队列满时会丢弃旧消息
- 建议在发布版本中使用 `SPDLOG_ACTIVE_LEVEL` 定义适当的日志级别
- 文件日志需要确保目录存在且有写入权限
- logger name 在全局必须唯一，重复的 name 会导致初始化失败

## 许可证

MIT License
