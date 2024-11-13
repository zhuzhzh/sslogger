#include "ssln/sslogger.h"

void some_function() {
    ssln::Stopwatch sw;
    
    // 做一些工作
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    SSLN_DEBUG_ELAPSED(sw, "First operation took {}");
    
    // 继续其他工作
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    SSLN_INFO_ELAPSED(sw, "Total time: {:.3}");
    
    // 重置计时器
    sw.reset();
    
    // 新的计时开始
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    SSLN_INFO_ELAPSED_US(sw, "New operation took {}");
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
}

void another_function() {
    ssln::Stopwatch sw;
    // 使用自定义消息
    SSLN_INFO_ELAPSED_US(sw, "from some to another operation took {}us");
    
    // 做一些耗时操作...
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    SSLN_INFO_ELAPSED_US(sw, "another operation took {}us");
}

int main(int argc, char const *argv[])
{
    ssln::Logger::Init();
    ssln::Stopwatch sw;
    some_function();
    another_function();
    SSLN_INFO_ELAPSED(sw, "two operations took {:012.9} sec");
    return 0;
}
