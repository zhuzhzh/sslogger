#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#include "ssln/sslogger.h"
#include <chrono>
#include <thread>
#include <vector>
#include <iomanip>
#include <iostream>

const size_t NUM_MESSAGES = 1000000;
const size_t NUM_THREADS = 10;
const size_t QUEUE_SIZE = 8192;
const size_t NUM_ITERATIONS = 3;

void bench_worker(size_t msg_count) {
    for (size_t i = 0; i < msg_count; ++i) {
        SPDLOG_INFO("Benchmark message #{}: Lorem ipsum dolor sit amet, consectetur adipiscing elit", i);
    }
}

void print_config(const std::string& title, bool is_async = true, spdlog::async_overflow_policy overflow_policy = spdlog::async_overflow_policy::block) {
    std::cout << "-------------------------------------------------\n";
    std::cout << "Messages     : " << NUM_MESSAGES << "\n";
    std::cout << "Threads      : " << NUM_THREADS << "\n";
    if (is_async) {
        std::cout << "Queue        : " << QUEUE_SIZE << " slots\n";
        const size_t MSG_SIZE = 272; // Approximate size of a log message in bytes
        const size_t QUEUE_MEMORY = QUEUE_SIZE * MSG_SIZE;
        std::cout << "Queue memory : " << QUEUE_SIZE << " x " << MSG_SIZE 
                  << " = " << (QUEUE_MEMORY / 1024) << " KB\n";
    }
    std::cout << "-------------------------------------------------\n\n";
    
    if (is_async) {
        std::cout << "*********************************\n";
        std::cout << "Queue Overflow Policy: " 
                  << (overflow_policy == spdlog::async_overflow_policy::block ? "block" : "overrun_oldest")
                  << "\n";
        std::cout << "*********************************\n";
    } else {
        std::cout << "*********************************\n";
        std::cout << "Synchronous File Logger\n";
        std::cout << "*********************************\n";
    }
}

void run_sync_benchmark(const std::string& title) {
    print_config(title, false);

    for (size_t iter = 0; iter < NUM_ITERATIONS; ++iter) {
        // Clear any existing loggers
        spdlog::shutdown();

        // Initialize synchronous file logger
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("bench_sync.log", true);
        auto logger = std::make_shared<spdlog::logger>("bench_logger", file_sink);
        logger->set_pattern("%v"); // Minimal pattern for maximum performance
        logger->set_level(spdlog::level::info);
        spdlog::set_default_logger(logger);

        // Create worker threads
        std::vector<std::thread> workers;
        const size_t msgs_per_thread = NUM_MESSAGES / NUM_THREADS;

        auto start = std::chrono::high_resolution_clock::now();

        // Launch worker threads
        for (size_t t = 0; t < NUM_THREADS; ++t) {
            workers.emplace_back(bench_worker, msgs_per_thread);
        }

        // Wait for all threads to complete
        for (auto& worker : workers) {
            worker.join();
        }

        // Calculate elapsed time and throughput
        auto end = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double>(end - start).count();
        double throughput = NUM_MESSAGES / elapsed;

        // Print results using cout
        std::cout << "Elapsed: " << std::fixed << std::setprecision(6) << elapsed 
                  << " secs     " << static_cast<size_t>(throughput) << "/sec\n";
        
        // Clean up and wait a bit before next iteration
        spdlog::shutdown();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    std::cout << "\n";
}

void run_benchmark(const std::string& title, spdlog::async_overflow_policy overflow_policy) {
    // Print configuration using cout instead of logger
    print_config(title, true, overflow_policy);

    for (size_t iter = 0; iter < NUM_ITERATIONS; ++iter) {
        // Clear any existing loggers
        spdlog::shutdown();

        // Initialize async logger with current policy
        spdlog::init_thread_pool(QUEUE_SIZE, NUM_THREADS);
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("bench_async.log", true);
        auto logger = std::make_shared<spdlog::async_logger>(
            "bench_logger",
            file_sink,
            spdlog::thread_pool(),
            overflow_policy
        );
        logger->set_pattern("%v"); // Minimal pattern for maximum performance
        logger->set_level(spdlog::level::info);
        spdlog::set_default_logger(logger);

        // Create worker threads
        std::vector<std::thread> workers;
        const size_t msgs_per_thread = NUM_MESSAGES / NUM_THREADS;

        auto start = std::chrono::high_resolution_clock::now();

        // Launch worker threads
        for (size_t t = 0; t < NUM_THREADS; ++t) {
            workers.emplace_back(bench_worker, msgs_per_thread);
        }

        // Wait for all threads to complete
        for (auto& worker : workers) {
            worker.join();
        }

        // Calculate elapsed time and throughput
        auto end = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double>(end - start).count();
        double throughput = NUM_MESSAGES / elapsed;

        // Print results using cout
        std::cout << "Elapsed: " << std::fixed << std::setprecision(6) << elapsed 
                  << " secs     " << static_cast<size_t>(throughput) << "/sec\n";
        
        // Clean up and wait a bit before next iteration
        spdlog::shutdown();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    std::cout << "\n";
}

int main() {
    try {
        // Run synchronous benchmark first
        run_sync_benchmark("Synchronous Mode");

        // Run async benchmarks
        run_benchmark("Blocking mode", spdlog::async_overflow_policy::block);
        run_benchmark("Overrun mode", spdlog::async_overflow_policy::overrun_oldest);
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
