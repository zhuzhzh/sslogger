#ifndef SSLN_SSLOGGER_H_
#define SSLN_SSLOGGER_H_

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/stopwatch.h>
#include <cstdlib>

namespace ssln {

// Verbosity预设
enum class Verbose {
    kLite,    // 只有消息
    kLow,     // 时间 + 消息
    kMedium,  // 时间 + 级别 + 位置
    kHigh,    // 时间 + 级别 + 线程 + 位置
    kFull,    // 完整时间 + 级别 + 线程 + 函数 + 行号
    kUltra    // 最详细格式
};

namespace detail {
    // 获取预设的pattern
    inline std::string get_pattern(Verbose ver) {
        switch (ver) {
            case Verbose::kLite:   return "%v";
            case Verbose::kLow:    return "[%H:%M:%S.%f] %v";
            case Verbose::kMedium: return "[%H:%M:%S.%f][%^%L%$][%@] %v";
            case Verbose::kHigh:   return "[%H:%M:%S.%f][%^%L%$][thread %t][%@] %v";
            case Verbose::kFull:   return "[%Y-%m-%d %H:%M:%S.%f][%^%L%$][thread %t][%!][%#] %v";
            case Verbose::kUltra:  return "[%Y-%m-%d %H:%M:%S.%F][%^%L%$][thread %t][%!][%@] %v";
            default:               return "[%Y-%m-%d %H:%M:%S.%f][%^%L%$][%t] %v";
        }
    }

    // 环境变量辅助函数
    inline std::string get_env_or(const char* name, const std::string& default_value) {
        const char* val = std::getenv(name);
        return val ? val : default_value;
    }

    inline spdlog::level::level_enum get_level_from_env(spdlog::level::level_enum default_level) {
        const char* level = std::getenv("SSLN_LOG_LEVEL");
        if (!level) return default_level;
        
        std::string level_str = level;
        if (level_str == "trace") return spdlog::level::trace;
        if (level_str == "debug") return spdlog::level::debug;
        if (level_str == "info")  return spdlog::level::info;
        if (level_str == "warn")  return spdlog::level::warn;
        if (level_str == "error") return spdlog::level::err;
        if (level_str == "fatal") return spdlog::level::critical;
        if (level_str == "off")   return spdlog::level::off;
        return default_level;
    }

    inline Verbose get_verbose_from_env(Verbose default_verbose) {
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

    // 添加线程池初始化标志
    inline bool& thread_pool_initialized() {
        static bool initialized = false;
        return initialized;
    }

    // 初始化线程池（如果尚未初始化）
    inline void init_thread_pool_once() {
        if (!thread_pool_initialized()) {
            const auto queue_size = std::stoi(get_env_or("SSLN_QUEUE_SIZE", "8192"));
            const auto n_threads = std::stoi(get_env_or("SSLN_THREADS", "1"));
            spdlog::init_thread_pool(queue_size, n_threads);
            thread_pool_initialized() = true;
        }
    }
} // namespace detail

// 控制台日志初始化
inline void init_console(
    spdlog::level::level_enum level = spdlog::level::info,
    Verbose verbose = Verbose::kMedium,
    const std::string& logger_name = "console") {
    
    try {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto logger = std::make_shared<spdlog::logger>(logger_name, console_sink);
        
        logger->set_level(detail::get_level_from_env(level));
        logger->set_pattern(detail::get_pattern(detail::get_verbose_from_env(verbose)));
        
        spdlog::register_logger(logger);
        spdlog::set_default_logger(logger);  // 设置为默认logger
    } catch (const spdlog::spdlog_ex& ex) {
        throw std::runtime_error(std::string("Logger initialization failed: ") + ex.what());
    }
}

// 同步文件日志初始化
inline void init_sync_file(
    const std::string& log_dir,
    const std::string& log_name,
    spdlog::level::level_enum level = spdlog::level::info,
    Verbose verbose = Verbose::kMedium,
    const std::string& logger_name = "sync_logger") {
    
    try {
        // 优先使用环境变量中的设置
        const auto final_dir = detail::get_env_or("SSLN_LOG_DIR", log_dir);
        const auto final_name = detail::get_env_or("SSLN_LOG_NAME", log_name);
        
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            final_dir + "/" + final_name);
            
        auto logger = std::make_shared<spdlog::logger>(logger_name, file_sink);
            
        // 应用环境变量设置
        logger->set_level(detail::get_level_from_env(level));
        logger->set_pattern(detail::get_pattern(detail::get_verbose_from_env(verbose)));
        
        spdlog::register_logger(logger);
        spdlog::set_default_logger(logger);  // 设置为默认logger
    } catch (const spdlog::spdlog_ex& ex) {
        throw std::runtime_error(std::string("Logger initialization failed: ") + ex.what());
    }
}

// 异步文件日志初始化（更新参数）
inline void init_async_file(
    const std::string& log_dir,
    const std::string& log_name,
    spdlog::level::level_enum level = spdlog::level::info,
    Verbose verbose = Verbose::kMedium, 
    const std::string& logger_name = "async_logger") {
    
    
    try {
        // 优先使用环境变量中的设置
        const auto final_dir = detail::get_env_or("SSLN_LOG_DIR", log_dir);
        const auto final_name = detail::get_env_or("SSLN_LOG_NAME", log_name);
        const auto flush_secs = std::stoi(detail::get_env_or("SSLN_FLUSH_SECS", "3"));
        
        detail::init_thread_pool_once();
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            final_dir + "/" + final_name);
            
        auto logger = std::make_shared<spdlog::async_logger>(
            logger_name, file_sink, spdlog::thread_pool(),
            spdlog::async_overflow_policy::block);
            
        logger->set_level(detail::get_level_from_env(level));
        logger->set_pattern(detail::get_pattern(detail::get_verbose_from_env(verbose)));
        
        spdlog::register_logger(logger);
        spdlog::set_default_logger(logger);  // 设置为默认logger
        spdlog::flush_every(std::chrono::seconds(flush_secs));
        
    } catch (const spdlog::spdlog_ex& ex) {
        throw std::runtime_error(std::string("Logger initialization failed: ") + ex.what());
    }
}

// Rotating文件日志初始化
inline void init_rotating_file(
    const std::string& log_dir,
    const std::string& log_name,
    size_t max_file_size,
    size_t max_files,
    spdlog::level::level_enum level = spdlog::level::info,
    Verbose verbose = Verbose::kMedium,
    const std::string& logger_name = "rotating_logger") {
    
    try {
        // 优先使用环境变量中的设置
        const auto final_dir = detail::get_env_or("SSLN_LOG_DIR", log_dir);
        const auto final_name = detail::get_env_or("SSLN_LOG_NAME", log_name);
        const auto final_max_size = std::stoull(detail::get_env_or("SSLN_MAX_FILE_SIZE", 
            std::to_string(max_file_size)));
        const auto final_max_files = std::stoull(detail::get_env_or("SSLN_MAX_FILES", 
            std::to_string(max_files)));
        
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            final_dir + "/" + final_name,
            final_max_size,
            final_max_files);
            
        auto logger = std::make_shared<spdlog::logger>(logger_name, file_sink);
            
        logger->set_level(detail::get_level_from_env(level));
        logger->set_pattern(detail::get_pattern(detail::get_verbose_from_env(verbose)));
        
        spdlog::register_logger(logger);
        spdlog::set_default_logger(logger);  // 设置为默认logger
    } catch (const spdlog::spdlog_ex& ex) {
        throw std::runtime_error(std::string("Logger initialization failed: ") + ex.what());
    }
}

// Stopwatch别名
using stopwatch = spdlog::stopwatch;

} // namespace ssln

#endif  // SSLN_SSLOGGER_H_