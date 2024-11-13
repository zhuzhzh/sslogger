#include "ssln/sslogger.h"
#include <spdlog/async.h>
#include <spdlog/details/log_msg.h>
#include <iostream>

/*!
 * @brief SSLN Logger namespace containing logging functionality
 */
namespace ssln {

std::shared_ptr<Logger> g_logger;

/*!
 * @brief Destructor for Logger class
 * 
 * Cleans up all logger resources including sinks and logger instances
 */
Logger::~Logger() {
    // Clean up all loggers
    loggers_.clear();
    async_logger_.reset();
    sync_logger_.reset();
    console_logger_.reset();
    file_sink_.reset();
    console_sink_.reset();
}

/*!
 * @brief Constructor for Logger class
 * 
 * Initializes the console logger with color support
 */
Logger::Logger() {
    // Initialize console logger
    console_sink_ = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_logger_ = std::make_shared<spdlog::logger>("console", console_sink_);
    spdlog::register_logger(console_logger_);
}

/*!
 * @brief Initializes the logging system with specified configuration
 * 
 * @param log_dir Directory where log files will be stored
 * @param log_file Name of the log file
 * @param async_mode Whether to use asynchronous logging
 * @param level Initial logging level
 * @param verbose Verbosity level for log formatting
 * @param allow_env_override Whether to allow environment variables to override settings
 */
void Logger::Init(const std::string& log_dir,
                 const std::string& log_file,
                 bool async_mode,
                 spdlog::level::level_enum level,
                 Verbose verbose,
                 bool allow_env_override) {
    if (!g_logger) {
        g_logger = std::make_shared<Logger>();
        std::atexit([]() {
            if (g_logger) {
                // First reset all loggers
                g_logger->loggers_.clear();
                
                // Clean up resources in specific order
                if (g_logger->async_logger_) {
                    spdlog::drop("async_logger");
                    g_logger->async_logger_.reset();
                }
                
                if (g_logger->sync_logger_) {
                    spdlog::drop("sync_logger");
                    g_logger->sync_logger_.reset();
                }
                
                if (g_logger->console_logger_) {
                    spdlog::drop("console");
                    g_logger->console_logger_.reset();
                }
                
                // Finally clean up sinks
                g_logger->file_sink_.reset();
                g_logger->console_sink_.reset();
                
                // Clean up global logger pointer
                g_logger.reset();
            }
            
            // Finally shutdown spdlog
            try {
                spdlog::shutdown();
            } catch (...) {
                // Ignore shutdown exceptions
            }
        });
        
        // Save initial configuration
        g_logger->config_.log_dir = log_dir;
        g_logger->config_.log_file = log_file;
        g_logger->config_.async_mode = async_mode;
        g_logger->config_.level = level;
        g_logger->config_.verbose = verbose;
        g_logger->config_.allow_env_override = allow_env_override;

        // Load environment variables if allowed
        if (allow_env_override) {
            g_logger->LoadFromEnv();
        }

        // Apply final configuration
        g_logger->SetAsyncMode(g_logger->config_.async_mode);
        g_logger->SetLevel(g_logger->config_.level);
        g_logger->SetVerbose(g_logger->config_.verbose);
        
        if (!g_logger->config_.log_file.empty()) {
            std::string full_path = g_logger->config_.log_dir + "/" + g_logger->config_.log_file;
            g_logger->SetFile(full_path);
        }
    }
}

/*!
 * @brief Sets the output file for logging
 * 
 * @param logfile Path to the log file
 * @param truncate Whether to truncate existing file
 * @return Reference to the Logger instance
 */
Logger& Logger::SetFile(const std::string& logfile, bool truncate) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    try {
        // Create buffered file sink
        file_sink_ = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            logfile, 
            truncate
        );
        
        if (config_.async_mode) {
            if (async_logger_) {
                async_logger_->flush();
                spdlog::drop("async_logger");
            }
            
            // Reinitialize thread pool if needed
            if (!spdlog::thread_pool()) {
                spdlog::init_thread_pool(config_.async_queue_size, config_.worker_threads);
            }
            
            // Create new async logger
            async_logger_ = std::make_shared<spdlog::async_logger>(
                "async_logger",
                file_sink_,
                spdlog::thread_pool(),
                spdlog::async_overflow_policy::block
            );
            
            // Configure async logger
            async_logger_->flush_on(spdlog::level::err);
            spdlog::register_logger(async_logger_);
            spdlog::flush_every(config_.flush_interval);
            
        } else {
            // Sync logger
            if (sync_logger_) {
                sync_logger_->flush();
                spdlog::drop("sync_logger");
            }
            
            sync_logger_ = std::make_shared<spdlog::logger>("sync_logger", file_sink_);
            spdlog::register_logger(sync_logger_);
            spdlog::set_default_logger(sync_logger_);
        }
        
        UpdateLoggersInternal();
        
    } catch (const spdlog::spdlog_ex& ex) {
        std::cerr << "Failed to create file sink: " << ex.what() << std::endl;
    }
    
    return *this;
}

/*!
 * @brief Internal method to update logger configurations
 * 
 * Updates all registered loggers with current configuration settings
 */
void Logger::UpdateLoggersInternal() {
    // Update console logger
    UpdateLogger(console_logger_);
    
    // Update file-based loggers
    if (config_.async_mode && async_logger_) {
        UpdateLogger(async_logger_);
    } else if (sync_logger_) {
        UpdateLogger(sync_logger_);
    }    

    for (auto& logger_pair : loggers_) {
        UpdateLogger(logger_pair.second);
    }
}

/*!
 * @brief Sets the logging level for all loggers
 * 
 * @param level The new logging level to set
 * @return Reference to the Logger instance
 */
Logger& Logger::SetLevel(spdlog::level::level_enum level) {
  config_.level = level;
  UpdateLoggers();
  return *this;
}

/*!
 * @brief Sets the asynchronous mode for logging
 * 
 * @param async_mode True for async logging, false for synchronous
 * @return Reference to the Logger instance
 */
Logger& Logger::SetAsyncMode(bool async_mode) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    config_.async_mode = async_mode;
    
    if (async_mode) {
        // Configure async logging
        spdlog::init_thread_pool(config_.async_queue_size, config_.worker_threads);
        
        if (file_sink_) {
            // Create buffered async logger
            async_logger_ = std::make_shared<spdlog::async_logger>(
                "async_logger",
                file_sink_,
                spdlog::thread_pool(),
                spdlog::async_overflow_policy::block
            );
            
            // Set buffer and flush policies
            async_logger_->flush_on(spdlog::level::err);  // Flush immediately on ERROR level
            async_logger_->set_pattern(config_.pattern_);
            async_logger_->set_level(config_.level);
            
            // Set periodic flush
            spdlog::flush_every(config_.flush_interval);
        }
    } else {
        // Shutdown async logging
        if (async_logger_) {
            async_logger_->flush();
            spdlog::drop("async_logger");
            async_logger_.reset();
        }
    }
    
    UpdateLoggersInternal();
    return *this;
}

/*!
 * @brief Environment variable configuration structure
 */
struct EnvVar {
    const char* name;              ///< Name of the environment variable
    std::function<void(const char*)> handler;  ///< Handler function for the variable
};

/*!
 * @brief Loads logger configuration from environment variables
 * 
 * Processes environment variables to override logger settings
 */
void Logger::LoadFromEnv() {
    // Move parsing function to lambda capture list before
    auto ParseLogLevel = [](const std::string& level) -> spdlog::level::level_enum {
        std::string upper_level = level;
        std::transform(upper_level.begin(), upper_level.end(), upper_level.begin(), ::toupper);

        if (upper_level == "OFF" || upper_level == "6") return SSLOGGER_OFF;
        if (upper_level == "FATAL" || upper_level == "5") return SSLOGGER_FATAL;
        if (upper_level == "ERROR" || upper_level == "4") return SSLOGGER_ERROR;
        if (upper_level == "WARN" || upper_level == "3") return SSLOGGER_WARN;
        if (upper_level == "INFO" || upper_level == "2") return SSLOGGER_INFO;
        if (upper_level == "DEBUG" || upper_level == "1") return SSLOGGER_DEBUG;
        if (upper_level == "TRACE" || upper_level == "0") return SSLOGGER_TRACE;

        std::cerr << "Invalid log level: " << level << ". Using default level (INFO)." << std::endl;
        return spdlog::level::info;
    };

    auto ParseVerbose = [](const std::string& verbose) -> Verbose {
        if (verbose == "lite") return Verbose::kLite;
        if (verbose == "low") return Verbose::kLow;
        if (verbose == "medium") return Verbose::kMedium;
        if (verbose == "high") return Verbose::kHigh;
        if (verbose == "full") return Verbose::kFull;
        if (verbose == "ultra") return Verbose::kUltra;
        return Verbose::kMedium;  // Add default return value
    };

    const EnvVar env_vars[] = {
        {"SSLN_LOG_DIR", [this](const char* value) { config_.log_dir = value; }},
        {"SSLN_LOG_FILE", [this](const char* value) { config_.log_file = value; }},
        {"SSLN_LOG_ASYNC", [this](const char* value) { 
            config_.async_mode = (std::string(value) == "1" || std::string(value) == "true"); 
        }},
        {"SSLN_LOG_LEVEL", [this, ParseLogLevel](const char* value) { 
            config_.level = ParseLogLevel(std::string(value)); 
        }},
        {"SSLN_LOG_VERBOSE", [this, ParseVerbose](const char* value) { 
            config_.verbose = ParseVerbose(std::string(value)); 
        }},
        {"SSLN_LOG_ASYNC_QUEUE_SIZE", [this](const char* value) { 
            config_.async_queue_size = std::stoull(value); 
        }},
        {"SSLN_LOG_WORKER_THREADS", [this](const char* value) { 
            config_.worker_threads = std::stoi(value); 
        }},
        {"SSLN_LOG_BUFFER_SIZE", [this](const char* value) { 
            config_.buffer_size = std::stoull(value); 
        }},
        {"SSLN_LOG_FLUSH_INTERVAL", [this](const char* value) { 
            config_.flush_interval = std::chrono::seconds(std::stoi(value)); 
        }},
    };

    for (const auto& env_var : env_vars) {
        if (const char* env_value = std::getenv(env_var.name)) {
            env_var.handler(env_value);
        }
    }
}

/*!
 * @brief Updates a specific logger instance with current configuration
 * 
 * @tparam LoggerType Type of the logger to update
 * @param logger Shared pointer to the logger instance
 */
template<typename LoggerType>
void Logger::UpdateLogger(std::shared_ptr<LoggerType>& logger) {
  if (logger) {
    logger->set_level(config_.level);
    logger->set_pattern(config_.pattern_);
  }
}

/*!
 * @brief Updates all registered loggers with current configuration
 * 
 * Thread-safe method to update all logger instances
 */
void Logger::UpdateLoggers() {
  std::lock_guard<std::mutex> lock(config_mutex_);
  UpdateLoggersInternal();
}

/*!
 * @brief Triggers registered callbacks for log messages
 * 
 * @param level Logging level of the message
 * @param file Source file where log was called
 * @param line Line number in source file
 * @param func Function name where log was called
 * @param message The log message content
 */
void Logger::TriggerCallbacks(spdlog::level::level_enum level, const char* file, int line,
  const char* func, const std::string& message) {
  spdlog::details::log_msg msg;
  msg.level = level;
  msg.time = std::chrono::system_clock::now();
  msg.source.filename = file;
  msg.source.line = line;
  msg.source.funcname = func;
  msg.payload = message;
  // Only iterate over registered callbacks
  for (size_t i = 0; i < callback_count_; ++i) {
      const auto& cb_pair = callbacks_[i];
      if (MatchesCondition(cb_pair.first, msg) && cb_pair.second) {
          try {
              cb_pair.second(msg);
          } catch (const std::exception& e) {
              std::cerr << "Error in callback: " << e.what() << std::endl;
          }
      }
  }
}

/*!
    * @brief Shuts down the logging system and cleans up resources
    * 
    * This method will:
    * 1. Clear all registered loggers
    * 2. Reset the global logger instance
    * 3. Shutdown spdlog
    */
void Logger::Shutdown() {
    if (g_logger) {
        // Clear all registered loggers
        g_logger->loggers_.clear();
        g_logger->async_logger_.reset();
        g_logger->sync_logger_.reset();
        g_logger->console_logger_.reset();
        g_logger->file_sink_.reset();
        g_logger->console_sink_.reset();
        
        // Reset global logger
        g_logger.reset();
    }
    
    // Shutdown spdlog
    try {
        spdlog::shutdown();
    } catch (...) {
        // Ignore shutdown exceptions
    }
}

/*!
 * @brief Sets the verbosity level for log formatting
 * 
 * @param ver Verbosity level to set
 * @return Reference to the Logger instance
 */
Logger& Logger::SetVerbose(Verbose ver) {
  switch (ver) {
    case Verbose::kLite: 
      config_.pattern_ = "%v"; 
      break;
    case Verbose::kLow: 
      config_.pattern_ = "[%H:%M:%S.%f] %v"; 
      break;
    case Verbose::kMedium: 
      config_.pattern_ = "[%H:%M:%S.%f][%^%L%$][%@] %v"; 
      break;
    case Verbose::kHigh: 
      config_.pattern_ = "[%H:%M:%S.%f][%^%L%$][thread %t][%@] %v"; 
      break;
    case Verbose::kFull: 
      config_.pattern_ = "[%Y-%m-%d %H:%M:%S.%f][%^%L%$][thread %t][%!][%#] %v"; 
      break;
    case Verbose::kUltra: 
      config_.pattern_ = "[%Y-%m-%d %H:%M:%S.%F][%^%L%$][thread %t][%!][%@] %v"; 
      break;
  }
  UpdateLoggers();
  return *this;
}

/*!
 * @brief Adds a new logger instance with specified configuration
 * 
 * @param name Name identifier for the logger
 * @param filename Path to the log file
 * @param daily Whether to use daily rotating file logger
 * @return Reference to the Logger instance
 */
Logger& Logger::AddLogger(const std::string& name, const std::string& filename, bool daily) {
  if (daily) {
    auto daily_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(filename, 0, 0);
    loggers_[name] = std::make_shared<spdlog::logger>(name, daily_sink);
  } else {
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename);
    loggers_[name] = std::make_shared<spdlog::logger>(name, file_sink);
  }
  UpdateLoggers();
  return *this;
}

/*!
 * @brief Adds a callback function for log message processing
 * 
 * @param condition Conditions under which the callback should be triggered
 * @param callback Function to be called when conditions are met
 * @throws std::runtime_error if maximum callback count is exceeded
 */
void Logger::AddCallback(const CallbackCondition& condition, CallbackFunction callback) {
  SSLN_ASSERT(callback_count_ < kMaxCallbacks, "Callback count exceeded maximum allowed callbacks");
  callbacks_[callback_count_++] = {condition, callback};
}

/*!
 * @brief Logs binary array data in hexadecimal format
 * 
 * @param level Logging level for the message
 * @param ptr Pointer to the binary data
 * @param size Size of the binary data in bytes
 * @param to_file Whether to write to file logger instead of console
 */
void Logger::LogArray(spdlog::level::level_enum level, const uint8_t* ptr, int size, bool to_file) {
  auto log_msg = fmt::format("Array data: {}", spdlog::to_hex(ptr, ptr + size));
  if (to_file && !loggers_.empty()) {
    loggers_.begin()->second->log(level, log_msg);
  } else if (config_.async_mode) {
    async_logger_->log(level, log_msg);
  } else {
    console_logger_->log(level, log_msg);
  }
  TriggerCallbacks(level, "", -1, "", log_msg);
}

/*!
 * @brief Checks if a log message matches the specified callback conditions
 * 
 * @param condition Callback conditions to check against
 * @param msg Log message to check
 * @return true if message matches all specified conditions
 */
bool Logger::MatchesCondition(const CallbackCondition& condition, const spdlog::details::log_msg& msg) {
  if (condition.level != SSLOGGER_OFF && condition.level != msg.level) {
    return false;
  }
  if (condition.file.has_value() && msg.source.filename != condition.file.value()) {
    return false;
  }
  if (condition.line.has_value() && msg.source.line != condition.line.value()) {
    return false;
  }
  if (condition.function.has_value() && msg.source.funcname != condition.function.value()) {
    return false;
  }
  if (condition.message.has_value() && 
      std::string(msg.payload.data(), msg.payload.size()).find(condition.message.value()) 
      == std::string::npos) {
    return false;
  }
  return true;
}

/*!
 * @brief Sets the asynchronous queue size
 * 
 * @param size New queue size
 * @return Reference to the Logger instance
 */
Logger& Logger::SetAsyncQueueSize(size_t size) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    config_.async_queue_size = size;
    if (config_.async_mode) {
        SetAsyncMode(true);  // Reconfigure async mode
    }
    return *this;
}

/*!
 * @brief Sets the number of worker threads
 * 
 * @param threads Number of worker threads
 * @return Reference to the Logger instance
 */
Logger& Logger::SetWorkerThreads(int threads) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    config_.worker_threads = threads;
    if (config_.async_mode) {
        SetAsyncMode(true);  // Reconfigure async mode
    }
    return *this;
}

/*!
 * @brief Sets the flush interval
 * 
 * @param interval New flush interval
 * @return Reference to the Logger instance
 */
Logger& Logger::SetFlushInterval(std::chrono::seconds interval) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    config_.flush_interval = interval;
    spdlog::flush_every(interval);
    return *this;
}

} // namespace ssln

