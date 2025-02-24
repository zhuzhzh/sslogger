#include "ssln/sslogger.h"
#include "quill/StopWatch.h"
#include <thread>

void some_function() {
    quill::StopWatchTsc sw;
    
    // Do some work
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    SSLN_INFO("First operation took {:.6}s", sw);
    
    // Continue with more work
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    SSLN_INFO("Total time: {:.3}s", sw);
    
    // Reset the timer
    sw.reset();
    
    // Start new timing
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    SSLN_INFO("New operation took {:.6}s", sw);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    SSLN_INFO("Detailed timing with source info: {} nanoseconds", sw.elapsed_as<std::chrono::nanoseconds>());
}

void another_function() {
    quill::StopWatchChrono sw;
    // Use custom message
    SSLN_INFO("From some to another operation took {:.6}s", sw);
    
    // Do some time-consuming operation...
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    SSLN_INFO("Another operation took {:.6}s", sw);
}

int main(int argc, char const *argv[]) {
    // Initialize console logger with full verbosity to see source info
    ssln::set_default_logger(ssln::console_logger);

    // Test both TSC and Chrono stopwatches
    {
        quill::StopWatchTsc sw;
        SSLN_INFO("Begin TSC StopWatch");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        SSLN_INFO("After 1s, elapsed: {:.6}s", sw);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        SSLN_INFO("After 500ms, elapsed: {}s", sw);
        SSLN_INFO("Elapsed nanoseconds: {}", sw.elapsed_as<std::chrono::nanoseconds>());
        SSLN_INFO("Elapsed seconds: {}", sw.elapsed_as<std::chrono::seconds>());
    }

    {
        quill::StopWatchChrono sw;
        SSLN_INFO("Begin Chrono StopWatch");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        SSLN_INFO("After 1s, elapsed: {:.6}s", sw);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        SSLN_INFO("After 500ms, elapsed: {}s", sw);
        SSLN_INFO("Elapsed nanoseconds: {}", sw.elapsed_as<std::chrono::nanoseconds>());
        SSLN_INFO("Elapsed seconds: {}", sw.elapsed_as<std::chrono::seconds>());
    }

    // Test in different functions
    some_function();
    another_function();

    return 0;
}
