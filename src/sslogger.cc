#include "ssln/sslogger.h"
#include <unistd.h>
#include <limits.h>
#include "quill/Backend.h"
#include "quill/Frontend.h"
#include "quill/Logger.h"
#include "quill/sinks/ConsoleSink.h"
#include "quill/sinks/FileSink.h"
#include "quill/sinks/RotatingFileSink.h"
#include <iostream>

namespace ssln {

// 定义全局 Logger 实例
quill::Logger* default_logger = nullptr;
quill::Logger* hybrid_logger = nullptr;
quill::Logger* axi_master_logger = nullptr;
quill::Logger* axi_slave_logger = nullptr;
quill::Logger* perf_logger = nullptr;
quill::Logger* console_logger = nullptr;
quill::Logger* daily_logger = nullptr;

quill::Logger* get_logger(const std::string& name) {
  return quill::Frontend::create_or_get_logger(name);
}

quill::Logger* set_default_logger(const std::string& name) {
  default_logger = get_logger(name);
  return default_logger;
} 

quill::Logger* set_default_logger(quill::Logger* logger) {
  default_logger = logger;
  return default_logger;
}

namespace detail {
    static std::mutex logger_paths_mutex;
    static std::unordered_map<std::string, std::string> logger_paths;
    
    std::string GetLoggerFilePath(const std::string& logger_name) {
        std::lock_guard<std::mutex> lock(logger_paths_mutex);
        auto it = logger_paths.find(logger_name);
        return it != logger_paths.end() ? it->second : std::string();
    }
    
    static void SetLoggerFilePath(const std::string& logger_name, const std::string& file_path) {
        std::lock_guard<std::mutex> lock(logger_paths_mutex);
        logger_paths[logger_name] = file_path;
    }
} // namespace detail

    //static std::string get_executable_path();
    //static std::string GetLogFilePath(const char* default_name);
    //static std::string GetEnvOr(const char* name, const std::string& default_value);
    //static quill::LogLevel GetLevelFromEnv(quill::LogLevel default_level);
    //static Verbose GetVerboseFromEnv(Verbose default_verbose);
    //static std::string GetPattern(Verbose ver);
    //static quill::PatternFormatterOptions GetFormatterOptions(Verbose verbose);

    static std::string get_executable_path() {
      char path[PATH_MAX];
      ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
      if (len != -1) {
        path[len] = '\0';
        return std::string(path);
      }
      return "ssln_default";
    }

    static std::string GetEnvOr(const char* name, const std::string& default_value) {
      const char* val = std::getenv(name);
      return val ? val : default_value;
    }

    static quill::LogLevel GetLevelFromEnv(quill::LogLevel default_level) {
      const char* level = std::getenv("SSLN_LOG_LEVEL");
      if (!level) return default_level;
      std::string level_str = level;
      if (level_str == "trace") return quill::LogLevel::TraceL3;
      if (level_str == "debug") return quill::LogLevel::Debug;
      if (level_str == "info")  return quill::LogLevel::Info;
      if (level_str == "warn")  return quill::LogLevel::Warning;
      if (level_str == "error") return quill::LogLevel::Error;
      if (level_str == "critical") return quill::LogLevel::Critical;
      if (level_str == "off")   return quill::LogLevel::None;
      return default_level;
    }

    static std::string GetLogFilePath(const char* default_name) {
      const char* dir = std::getenv("SSLN_LOG_DIR");
      const char* name = std::getenv("SSLN_LOG_NAME");
      std::string file_name = name ? name : default_name;
      std::string base_path = dir ? (std::string(dir) + "/" + file_name) : file_name;
      
      if (!dir) {
        std::string exe_path = get_executable_path();
        size_t last_slash = exe_path.find_last_of('/');
        if (last_slash != std::string::npos) {
          base_path = exe_path.substr(0, last_slash + 1) + file_name;
        }
      }
      return base_path;
    }

    static Verbose GetVerboseFromEnv(Verbose default_verbose) {
      const char* verb = std::getenv("SSLN_VERBOSITY");
      if (!verb) return default_verbose;
      std::string verb_str = verb;
      if (verb_str == "lite")   return Verbose::kLite;
      if (verb_str == "low")    return Verbose::kLow;
      if (verb_str == "medium") return Verbose::kMedium;
      if (verb_str == "high")   return Verbose::kHigh;
      if (verb_str == "full")   return Verbose::kFull;
      if (verb_str == "ultra")  return Verbose::kUltra;
      return default_verbose;
    }

    std::string GetPattern(Verbose ver) {
      switch (ver) {
        case Verbose::kLite:   return "%(message)";
        case Verbose::kLow:    return "[%(time)] %(message)";
        case Verbose::kMedium: return "[%(time)] [%(log_level)] [%(file_name):%(line_number)] %(message)";
        case Verbose::kHigh:   return "[%(time)] [%(log_level)] [%(thread_id)] [%(file_name):%(line_number)] %(message)";
        case Verbose::kFull:   return "[%(time)] [%(log_level)] [%(thread_id)] [%(caller_function)] [%(file_name):%(line_number)] %(message)";
        case Verbose::kUltra:  return "[%(time)] [%(log_level)] [%(thread_id)] [%(caller_function)] [%(file_name):%(line_number)] %(message)";
        default:               return "[%(time)] [%(log_level)] [%(thread_id)] %(message)";
      }
    }

    static quill::PatternFormatterOptions GetFormatterOptions(Verbose verbose) {
      return quill::PatternFormatterOptions{
        GetPattern(GetVerboseFromEnv(verbose)),
        "%H:%M:%S.%Qns",
        quill::Timezone::LocalTime
      };
    }

  // 封装的 Logger 创建函数
  quill::Logger* SetupFileLogger(const char* log_file, const std::string& logger_name, 
                                        Verbose verbose, 
                                        quill::LogLevel level, 
                                        bool append_date){ 
    std::string file_path = GetLogFilePath(log_file);
    detail::SetLoggerFilePath(logger_name, file_path);
    quill::FileSinkConfig file_cfg;
    file_cfg.set_open_mode('w');
    file_cfg.set_filename_append_option(append_date ? quill::FilenameAppendOption::StartDateTime : quill::FilenameAppendOption::None);
    auto file_sink = quill::Frontend::create_or_get_sink<quill::FileSink>(file_path, file_cfg);
    auto logger = quill::Frontend::create_or_get_logger(
        logger_name, std::move(file_sink), GetFormatterOptions(verbose));
    logger->set_log_level(GetLevelFromEnv(level));
    return logger;
  }

  quill::Logger* SetupConsoleLogger(const std::string& logger_name, 
                                          Verbose verbose, 
                                          quill::LogLevel level) {
    auto console_sink = quill::Frontend::create_or_get_sink<quill::ConsoleSink>("console_sink");
    auto logger = quill::Frontend::create_or_get_logger(
        logger_name, std::move(console_sink), GetFormatterOptions(verbose));
    logger->set_log_level(GetLevelFromEnv(level));
    return logger;
  }

  quill::Logger* SetupRotatingLogger(const char* log_file, size_t max_file_size, size_t max_files,
                                            const std::string& logger_name, 
                                            Verbose verbose, 
                                            quill::LogLevel level) {
    std::string file_path = GetLogFilePath(log_file);
    detail::SetLoggerFilePath(logger_name, file_path);
    quill::RotatingFileSinkConfig rotating_cfg;
    rotating_cfg.set_open_mode('w');
    rotating_cfg.set_filename_append_option(quill::FilenameAppendOption::StartDateTime);
    rotating_cfg.set_rotation_max_file_size(max_file_size);
    rotating_cfg.set_max_backup_files(max_files);
    auto rotating_sink = quill::Frontend::create_or_get_sink<quill::RotatingFileSink>(
        file_path, rotating_cfg);
    auto logger = quill::Frontend::create_or_get_logger(
        logger_name, std::move(rotating_sink), GetFormatterOptions(verbose));
    logger->set_log_level(GetLevelFromEnv(level));
    return logger;
  }

  quill::Logger* SetupPerfLogger(const char* log_file, const std::string& logger_name,
                                    quill::LogLevel level, 
                                    Verbose verbose) {
    try {
      std::string file_path = GetLogFilePath(log_file);
      detail::SetLoggerFilePath(logger_name, file_path);
      auto perf_sink = quill::Frontend::create_or_get_sink<quill::FileSink>(
          file_path,
          []() {
            quill::FileSinkConfig cfg;
            cfg.set_open_mode('w');
            return cfg;
          }(),
          quill::FileEventNotifier{});

      auto logger = quill::Frontend::create_or_get_logger(
          logger_name, std::move(perf_sink), quill::PatternFormatterOptions{"%(message)"});
      logger->set_log_level(GetLevelFromEnv(level));

      if (logger_name == "perf_logger") {
        perf_logger = logger;
      }
      
      return logger;
    } catch (const std::exception& ex) {
      throw std::runtime_error(std::string("Logger initialization failed: ") + ex.what());
    }
  }

namespace detail {
  // 静态日志初始化类
  struct LoggerInitializer {
    LoggerInitializer() {
      quill::Backend::start(); // 启动 quill 后台线程

      // 初始化所有 Logger
      hybrid_logger = SetupFileLogger("hybrid.log", "hybrid_logger");
      axi_master_logger = SetupFileLogger("axi_master.log", "axi_master_logger");
      axi_slave_logger = SetupFileLogger("axi_slave.log", "axi_slave_logger");
      perf_logger = SetupPerfLogger("perf.log", "perf_logger");
      console_logger = SetupConsoleLogger("console_logger");
      daily_logger = SetupRotatingLogger("daily.log", 1024 * 1024 * 10, 5, "daily_logger");

      // 设置默认 Logger
      default_logger = hybrid_logger; // 默认使用hybrid_logger
    }

  };

  // 全局静态实例
  static LoggerInitializer initializer;
} // namespace detail


} // namespace ssln
