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

void Logger::LogArrayToFile(Level level, const char* file, int line, const uint8_t* ptr, size_t sz) {
    if (level <= current_level_) {
        std::vector<uint8_t> vec(ptr, ptr + sz);
        vgp::Logger::GetInstance().LogToFile(level, file, line, "{}", vec);
    }
}

void logf_array(Logger::Level level, const uint8_t* ptr, size_t sz) {
    vgp::Logger::GetInstance().LogArrayToFile(level, __FILE__, __LINE__, ptr, sz);
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

void Logger::log_impl(const source_location& loc, Level level, const std::string& msg)
{
    static const char* level_strings[] = {"OFF", "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
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

    std::lock_guard<std::mutex> lock(mutex_);
    if (log_file_.is_open()) {
        log_file_ << formatted_msg << std::endl;
    } else {
        fmt::print("{}\n", formatted_msg);
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

