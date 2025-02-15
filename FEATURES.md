# Logging Library Feature Requirements

## 1. Core Logging Features

### 1.1 Log Levels
- Support standard severity levels: TRACE, DEBUG, INFO, WARN, ERROR, FATAL/CRITICAL
- Allow runtime log level adjustment
- Support compile-time log level filtering
- Allow setting default log level via environment variable

### 1.2 Output Formats
Six verbosity presets for different use cases:
- `kLite`: Message only
  ```
  Hello world
  ```
- `kLow`: Basic timestamp + message
  ```
  [23:59:59.123] Hello world
  ```
- `kMedium`: Timestamp + level + source location + message
  ```
  [23:59:59.123][INFO][file.cpp:42] Hello world
  ```
- `kHigh`: Timestamp + level + thread ID + source location + message
  ```
  [23:59:59.123][INFO][thread 1234][file.cpp:42] Hello world
  ```
- `kFull`: Full timestamp + level + thread + function + line + message
  ```
  [2024-01-20 23:59:59.123][INFO][thread 1234][function_name][file.cpp:42] Hello world
  ```
- `kUltra`: High precision timestamp + all details
  ```
  [2024-01-20 23:59:59.123456789][INFO][thread 1234][function_name][file.cpp:42] Hello world
  ```

### 1.3 Output Destinations
Support multiple initialization methods:
- Console logging with colored output
- Synchronous file logging
- Asynchronous file logging with configurable queue
- Rotating file logging with size limits
- Support multiple sinks simultaneously

## 2. Advanced Features

### 2.1 Performance Features
- Asynchronous logging with configurable thread pool
- Configurable queue size for async mode
- Overflow policies (block vs overrun)
- Automatic periodic flush
- High-precision timestamps
- Compile-time filtering of log levels

### 2.2 Binary Data Logging
- Hex dump formatting for binary data
- Configurable bytes per line
- Support for both arrays and vectors
- Proper formatting and alignment

### 2.3 Performance Measurement
- Built-in stopwatch functionality
- High precision time measurements
- Flexible time unit formatting
- Easy to use in logging statements

## 3. Configuration Options

### 3.1 Environment Variables
- `SSLN_LOG_LEVEL`: Set logging level
- `SSLN_VERBOSITY`: Set output format preset
- `SSLN_LOG_DIR`: Set log file directory
- `SSLN_LOG_NAME`: Set log file name
- `SSLN_QUEUE_SIZE`: Set async queue size
- `SSLN_THREADS`: Set number of async threads
- `SSLN_FLUSH_SECS`: Set flush interval
- `SSLN_MAX_FILE_SIZE`: Set max file size for rotation
- `SSLN_MAX_FILES`: Set max number of backup files

### 3.2 Runtime Configuration
- Dynamic log level adjustment
- Pattern layout changes
- Sink management (add/remove)
- Flush control
- Thread pool management

## 4. Implementation Requirements

### 4.1 Thread Safety
- All public interfaces must be thread-safe
- Support concurrent logging from multiple threads
- Safe initialization and shutdown
- Thread-safe sink management

### 4.2 Resource Management
- RAII-based resource handling
- Clean shutdown procedure
- Proper cleanup of file handles
- Memory leak prevention

### 4.3 Error Handling
- Exception safety
- Graceful error recovery
- Proper error reporting
- Fallback mechanisms

### 4.4 Performance Goals
Benchmark targets (million messages test):
- Synchronous mode: ~50K msg/sec
- Async blocking mode: ~500K msg/sec
- Async overrun mode: ~1.7M msg/sec

## 5. API Design

### 5.1 Initialization Functions
```cpp
void InitConsole(level, verbose, logger_name);
void InitAsyncFile(log_dir, log_name, level, verbose, logger_name);
void InitRotatingFile(log_dir, log_name, max_size, max_files, level, verbose, logger_name);
```

### 5.2 Logging Macros
Should support both stream-style and printf-style formatting:
```cpp
LOGGER_INFO("Value: {}", 42);
LOGGER_DEBUG("Array: {}", binary_data);
LOGGER_WARN("Time elapsed: {}", stopwatch);
```

### 5.3 Utility Functions
- Stopwatch/timer functions
- Binary data formatting
- Pattern generation
- Environment variable handling

## 6. Testing Requirements

### 6.1 Unit Tests
- Basic logging functionality
- Format patterns
- Verbosity levels
- Binary logging
- Stopwatch accuracy
- Environment variables
- Error conditions

### 6.2 Performance Tests
- Throughput measurements
- Latency tests
- Memory usage
- Thread scaling
- File I/O performance 