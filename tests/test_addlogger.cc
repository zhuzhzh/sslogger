#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <ssln/sslogger.h>
#include <fstream>
#include <regex>

using ssln::g_logger;

class AddLoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化日志系统
        ssln::Logger::Init(".", "", false, SSLOGGER_TRACE);
        // 确保 g_logger 已经被正确初始化
        ASSERT_NE(g_logger, nullptr);
    }

    void TearDown() override {
    }

    // 辅助函数：检查文件是否包含特定内容
    bool FileContains(const std::string& filename, const std::string& text) {
        std::ifstream file(filename);
        if (!file.is_open()) return false;
        
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        file.close();
        return content.find(text) != std::string::npos;
    }
};

TEST_F(AddLoggerTest, BasicLogging) {
    // 测试基本日志输出
    int i = 32;
    SSLN_DEBUG("this is one debug msg {}", i);
    
    // 添加新的日志器
    g_logger->AddLogger("test_logger", "logger.log");
    SSLN_INFO_TO("test_logger", "this is one info msg to logger.log");
    
    // 等待一小段时间确保日志被写入
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 验证日志文件内容
    EXPECT_TRUE(FileContains("logger.log", "this is one info msg to logger.log"));
}

TEST_F(AddLoggerTest, MultipleLoggers) {
    // 测试多个日志器
    g_logger->AddLogger("logger1", "log1.log");
    g_logger->AddLogger("logger2", "log2.log");
    
    SSLN_INFO_TO("logger1", "Message to logger1");
    SSLN_INFO_TO("logger2", "Message to logger2");
    
    // 等待日志写入
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 验证每个日志文件的内容
    EXPECT_TRUE(FileContains("log1.log", "Message to logger1"));
    EXPECT_TRUE(FileContains("log2.log", "Message to logger2"));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}