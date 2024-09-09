#define VHLOGGER_ENABLE_CB

#include "vhlogger/vhlogger.h"
#include <iostream>
#include <cassert>


void test_callback() {
    auto logger = vgp::Logger::GetInstance();
    
    // 设置日志级别和格式
    logger->SetLogLevel(VHLOGGER_DEBUG)
          .SetFormat(vgp::Logger::Format::kMedium);

    // 用于测试的回调函数
    bool callback_triggered = false;
    auto test_callback = [&callback_triggered](const spdlog::details::log_msg& msg) {
        std::cout << "Callback triggered for message: " << std::string(msg.payload.data(), msg.payload.size()) << std::endl;
        callback_triggered = true;
    };

    // 添加回调条件
    vgp::Logger::CallbackCondition condition;
    condition.level = VHLOGGER_INFO;
    condition.message = "test callback";
    logger->AddCallback(condition, test_callback);

    // 记录一些日志消息
    VGP_DEBUG("This is a debug message");
    VGP_INFO("This is an info message");
    VGP_INFO("This message should trigger the test callback");

    // 检查回调是否被触发
    assert(callback_triggered);
    std::cout << "Callback test passed!" << std::endl;
}

int main() {
    test_callback();
    return 0;
}
