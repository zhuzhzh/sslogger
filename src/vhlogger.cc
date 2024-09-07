#include "vhlogger/vhlogger.h"

namespace vgp {

  Logger& Logger::GetInstance() {
    static Logger instance;
    return instance;
  }

#ifdef CPP17_OR_GREATER
  Logger::CallbackId Logger::AddCallback(CallbackFunction func, Level level, 
    std::optional<std::string> message,
    std::optional<std::string> file,
    std::optional<int> line) 
#else
    Logger::CallbackId Logger::AddCallback(CallbackFunction func, Level level, 
      tl::optional<std::string> message,
      tl::optional<std::string> file,
      tl::optional<int> line) 
#endif
    {
      std::lock_guard<std::mutex> lock(mutex_);
      CallbackId id = next_callback_id_++;
      callbacks_[id] = {func, static_cast<int>(level), message, file, line};
      return id;
    }

  bool Logger::RemoveCallback(CallbackId id) {
    std::lock_guard<std::mutex> lock(mutex_);
    return callbacks_.erase(id) > 0;
  }

#ifdef CPP17_OR_GREATER
  void Logger::ClearCallbacks(Level level, 
    std::optional<std::string> message,
    std::optional<std::string> file,
    std::optional<int> line) 
#else
    void Logger::ClearCallbacks(Level level, 
      tl::optional<std::string> message,
      tl::optional<std::string> file,
      tl::optional<int> line) 
#endif
    {
      std::lock_guard<std::mutex> lock(mutex_);
      auto it = callbacks_.begin();
      while (it != callbacks_.end()) {
        const auto& callback = it->second;
        if (callback.level == static_cast<int>(level) &&
          (!message || callback.message == message) &&
          (!file || callback.file == file) &&
          (!line || callback.line == line)) {
          it = callbacks_.erase(it);
        } else {
          ++it;
        }
      }
    }

  void Logger::EnqueueLogMessage(LogMessage&& msg) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (!shutdown_requested_.load()) {
        log_queue_.push(std::move(msg));
        queue_cv_.notify_one();
    } else {
        // Optionally, handle messages that come in during shutdown
        // For example, you could write them directly or discard them
        LogImpl(msg.loc, msg.level, "Message discarded during shutdown: " + msg.message, msg.to_file);
    }
  }

  void Logger::WorkerThread() {
    while (true) {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      queue_cv_.wait(lock, [this] { return !log_queue_.empty() || shutdown_requested_.load(); });

      while (!log_queue_.empty()) {
        auto msg = std::move(log_queue_.front());
        log_queue_.pop();
        lock.unlock();
        LogImpl(msg.loc, msg.level, msg.message, msg.to_file);
        lock.lock();
      }

      if (shutdown_requested_.load() && log_queue_.empty()) {
        shutdown_completed_.store(true);
        shutdown_cv_.notify_one();
        break;
      }
    }
  }

  void Logger::Shutdown() {
    {
      std::lock_guard<std::mutex> lock(queue_mutex_);
      if (shutdown_requested_.exchange(true)) {
        // Shutdown already in progress
        return;
      }
    }

    // Notify the worker thread to process remaining messages
    queue_cv_.notify_one();

    // Wait for the worker thread to finish processing
    {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      shutdown_cv_.wait(lock, [this] { return shutdown_completed_.load(); });
    }

    if (worker_thread_.joinable()) {
      worker_thread_.join();
    }

    if (log_file_.is_open()) {
      log_file_.close();
    }
  }

  void Logger::TriggerCallbacks(const LogContext& context) {
    for (const auto& callback_pair: callbacks_) {
      const auto & id = callback_pair.first;
      const auto & callback = callback_pair.second;

      if (context.level == callback.level &&
        (!callback.message || *callback.message == context.message) &&
        (!callback.file || *callback.file == context.file) &&
        (!callback.line || *callback.line == context.line)) {
        callback.func(context);
      }
    }
  }

  void Logger::LogArray(const logger_loc& loc, Level level, bool to_file, const uint8_t* ptr, size_t sz) {
    if (level <= current_level_) {
      std::vector<uint8_t> vec(ptr, ptr + sz);
      Log(loc.loc_, level, to_file, "{}", vec);
    }
  }

  Logger::Level Logger::ParseLogLevel(const std::string& level) {
    // Convert to uppercase for case-insensitive comparison
    std::string upper_level = level;
    std::transform(upper_level.begin(), upper_level.end(), upper_level.begin(), ::toupper);

    if (upper_level == "OFF" || upper_level == "0") return Level::OFF;
    if (upper_level == "FATAL" || upper_level == "1") return Level::FATAL;
    if (upper_level == "ERROR" || upper_level == "2") return Level::ERROR;
    if (upper_level == "WARN" || upper_level == "3") return Level::WARN;
    if (upper_level == "INFO" || upper_level == "4") return Level::INFO;
    if (upper_level == "DEBUG" || upper_level == "5") return Level::DEBUG;
    if (upper_level == "TRACE" || upper_level == "6") return Level::TRACE;

    // If it's not a recognized string, try to parse it as a number
    try {
      int level_num = std::stoi(level);
      if (level_num >= 0 && level_num <= 6) {
        return static_cast<Level>(level_num);
      } else if (level_num > 6) {
        return static_cast<Level>(6);
      } else if (level_num < 0 ) {
        return static_cast<Level>(0);
      }
    } catch (const std::exception&) {
      // If parsing as a number fails, we'll fall through to the default case
    }

    // If we get here, the input was invalid. Log a warning and return the default level.
    std::cerr << "Invalid log level: " << level << ". Using default level (INFO)." << std::endl;
    return Level::INFO;
  }

  Logger& Logger::SetLogLevel(Level verbose) {
    current_level_.store(verbose, std::memory_order_relaxed);
    return *this;
  }

  Logger& Logger::SetLogLevel(int verbose) {
    current_level_.store(static_cast<Level>(verbose), std::memory_order_relaxed);
    return *this;
  }

  Logger& Logger::SetFormat(Format format) { 
    std::lock_guard<std::mutex> lock(mutex_);
    format_ = format; 
    return *this;
  }

  Logger& Logger::SetLogFile(const std::string& filename, bool append) {
    std::lock_guard<std::mutex> lock(mutex_);
    file_name_ = filename;
    if (log_file_.is_open()) {
      log_file_.close();
    }
    std::ios_base::openmode mode = std::ios_base::out;
    if (append) {
      mode |= std::ios_base::app;
    }

    log_file_.open(filename, mode);
    if (!log_file_) {
      throw std::runtime_error("Failed to open log file: " + filename);
    }
    return *this;
  }

  const Logger::Format Logger::GetFormat() const {
    return format_;
  }
  const std::string Logger::GetLogFile() const {
    return file_name_;
  }
  const Logger::Level Logger::GetLogLevel() const {
    return current_level_;
  }

  std::string Logger::FormatMessage(Level level, const char* file, int line, const std::string& message) {
    std::string level_str;
    switch (static_cast<int>(level)) {
    case 1: level_str = "FATAL"; break;
    case 2: level_str = "ERROR"; break;
    case 3: level_str = "WARN"; break;
    case 4: level_str = "INFO"; break;
    case 5: level_str = "DEBUG"; break;
    case 6: level_str = "TRACE"; break;
    default: level_str = "TRACE"; break;
    }

    auto now = std::chrono::system_clock::now();

    switch (format_) {
    case Format::kLite:
      return message;
    case Format::kMedium:
      return fmt::format("[{}][{}] {}", level_str, fmt::format("{:%Y-%m-%d %H:%M:%S}", now), message);
    case Format::kFull:
    default:
      return fmt::format("[{}][{}][{}:{}] {}", level_str, fmt::format("{:%Y-%m-%d %H:%M:%S}", now), file, line, message);
    }
  }

  void Logger::LogImpl(const source_location& loc, Level level, const std::string& msg, bool to_file)
  {
    static const char* level_strings[] = {"OFF", "FATAL", "ERROR", "WARN", "INFO", "DEBUG", "TRACE"};
    std::string level_str = level_strings[static_cast<int>(level)];

    auto now = std::chrono::system_clock::now();

    std::string formatted_msg;
    switch (format_) {
    case Format::kLite:
      formatted_msg = msg;
      break;
    case Format::kMedium:
      formatted_msg = fmt::format("[{}][{}] {}", level_str, fmt::format("{:%Y-%m-%d %H:%M:%S}", now), msg);
      break;
    case Format::kFull:
    default:
      formatted_msg = fmt::format("[{}][{}][{}:{}:{}] {}", 
        level_str, 
        fmt::format("{:%Y-%m-%d %H:%M:%S}", now), 
        loc.file_name(), 
        loc.line(), 
        loc.function_name(), 
        msg);
      break;
    }

    {
      std::lock_guard<std::mutex> lock(mutex_);
      if(to_file) {
        if (log_file_.is_open()) {
          log_file_ << formatted_msg << std::endl;
        } else {
          fmt::print("{}\n", formatted_msg);
        }
      } else {
        fmt::print("{}\n", formatted_msg);
      }
    }
    LogContext context{static_cast<int>(level), loc.file_name(), loc.line(), msg};
    TriggerCallbacks(context);
  }

  const char* Logger::ToString(Level level) {
    switch (level) {
    case Level::TRACE: return "TRACE";
    case Level::DEBUG: return "DEBUG";
    case Level::INFO: return "INFO";
    case Level::WARN: return "WARN";
    case Level::ERROR: return "ERROR";
    case Level::FATAL: return "FATAL";
    case Level::OFF: return "OFF";
    default: return "UNKNOWN";
    }
  }

} // namespace vgp

