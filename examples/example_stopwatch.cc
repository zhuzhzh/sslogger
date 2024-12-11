#include "ssln/sslogger.h"

void some_function() {
    spdlog::stopwatch sw;
    
    // 做一些工作
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    spdlog::info("First operation took {}", sw);
    
    // 继续其他工作
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    spdlog::info("Total time: {:.3}", sw);
    
    // 重置计时器
    sw.reset();
    
    // 新的计时开始
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    spdlog::info("New operation took {}", sw);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    SPDLOG_INFO("Detailed timing with source info: {:.3f} seconds", sw.elapsed().count());
}

void another_function() {
    spdlog::stopwatch sw;
    // 使用自定义消息
    spdlog::info("from some to another operation took {}", sw);
    
    // 做一些耗时操作...
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    spdlog::info("another operation took {}", sw);
}

int main(int argc, char const *argv[])
{
    spdlog::stopwatch sw;
    some_function();
    another_function();
    spdlog::info("two operations took {:012.9} sec", sw);
    return 0;
}
