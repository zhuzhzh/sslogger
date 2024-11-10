#define SSLOGGER_ENABLE_CB

#include "ssln/sslogger.h"
#include <iostream>
#include <cassert>

using ssln::g_logger;

void test_callback() {
    ssln::Logger::Init();
    // 设置日志级别和格式
    g_logger->SetLevel(SSLOGGER_DEBUG)
          .SetVerbose(ssln::Logger::Verbose::kMedium);

    // 用于测试的回调函数
    bool callback_triggered = false;
    auto test_callback = [&callback_triggered](const spdlog::details::log_msg& msg) {
        std::cout << "Callback triggered for message: " << std::string(msg.payload.data(), msg.payload.size()) << std::endl;
        callback_triggered = true;
    };

    // 添加回调条件
    ssln::Logger::CallbackCondition condition;
    condition.level = SSLOGGER_INFO;
    condition.message = "test callback";
    g_logger->AddCallback(condition, test_callback);

    // 记录一些日志消息
    SSLN_DEBUG("This is a debug message");
    SSLN_INFO("This is an info message");
    SSLN_INFO("This message should trigger the test callback");

    // 检查回调是否被触发
    assert(callback_triggered);
    std::cout << "Callback test passed!" << std::endl;
}

int main() {
    test_callback();
    return 0;
}
