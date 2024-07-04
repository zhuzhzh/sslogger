#include "vhlogger/vhlogger.h"

namespace vgp {

Logger& Logger::GetInstance() {
  static Logger instance;
  return instance;
}

Logger::CallbackId Logger::AddCallback(CallbackFunction func, int level, 
                         std::optional<std::string> message,
                         std::optional<std::string> file,
                         std::optional<int> line) {
  std::lock_guard<std::mutex> lock(mutex_);
  CallbackId id = next_callback_id_++;
  callbacks_[id] = {func, level, message, file, line};
  return id;
}

bool Logger::RemoveCallback(CallbackId id) {
  std::lock_guard<std::mutex> lock(mutex_);
  return callbacks_.erase(id) > 0;
}

void Logger::ClearCallbacks(int level, 
                            std::optional<std::string> message,
                            std::optional<std::string> file,
                            std::optional<int> line) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = callbacks_.begin();
  while (it != callbacks_.end()) {
    const auto& callback = it->second;
    if (callback.level == level &&
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
  for (const auto& [id, callback] : callbacks_) {
    if (context.level == callback.level &&
        (!callback.message || *callback.message == context.message) &&
        (!callback.file || *callback.file == context.file) &&
        (!callback.line || *callback.line == context.line)) {
      callback.func(context);
    }
  }
}

void Logger::SetFormat(Format format) { format_ = format; }

void Logger::SetLogFile(const std::string& filename, bool append) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (log_file_.is_open()) {
    log_file_.close();
  }
  log_file_.open(filename, append ? std::ios_base::app : std::ios_base::out);
}

std::string Logger::FormatMessage(int level, const char* file, int line, const std::string& message) {
  std::string level_str;
  switch (level) {
    case 1: level_str = "ERROR"; break;
    case 2: level_str = "WARN"; break;
    case 3: level_str = "INFO"; break;
    case 4: level_str = "DEBUG"; break;
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
}