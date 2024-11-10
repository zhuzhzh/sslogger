# VHLogger

VHLogger 是一个基于 spdlog 的 C++ 日志库封装，提供了灵活和强大的日志功能。

## 主要特性

- 支持控制台和文件日志输出
- 异步和同步日志模式
- 可自定义日志级别和格式
- 支持多个日志实例
- 环境变量配置
- 回调函数支持

## 使用方法

### 基本用法

```cpp
#include "vhlogger.h"

int main() {
    ssln::Logger::Init(".", "test.log", false, SSLOGGER_INFO, ssln::Logger::Verbose::kMedium, false);

    // 使用日志宏
    SSLN_INFO("Hello, VHLogger!");
    SSLN_ERROR("An error occurred: {}", "File not found");

    return 0;
}
```

### 异步模式

```cpp
ssln::Logger::Init(".", "test.log", true, SSLOGGER_INFO, ssln::Logger::Verbose::kMedium, true);
```


### 添加自定义日志实例

```cpp
g_logger->AddLogger("custom_logger", "custom.log");
SSLN_INFO_TO("custom_logger", "This is a custom log message");
```

### 使用回调函数

```cpp
ssln::Logger::CallbackCondition condition;
condition.level = SSLOGGER_ERROR;
condition.message = "critical";

g_logger->AddCallback(condition, [](const spdlog::details::log_msg& msg) {
    // 处理日志消息
    std::cout << "Callback triggered: " << msg.payload << std::endl;
});
```

### 日志数组数据

```cpp
uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
SSLN_LOG_ARRAY(SSLOGGER_DEBUG, data, sizeof(data));
```

## 环境变量配置

SSLN_LOG_LEVEL: 设置日志级别 (OFF, FATAL, ERROR, WARN, INFO, DEBUG, TRACE)
SSLN_LOG_VERBOSE: 设置日志详细程度 (lite, low, medium, high, full, ultra)
SSLN_LOG_FILE: 设置日志文件名
SSLN_LOG_DIR: 设置日志文件目录
SSLN_LOG_ASYNC: 设置异步模式 (true or false)

## 注意事项

- 确保在使用前正确初始化 Logger 实例
- 异步模式可能会在程序突然终止时丢失一些日志消息
- 使用文件日志时，确保应用程序有写入权限

## 依赖

- spdlog
- fmt
- tl::optional
