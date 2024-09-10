#include <vhlogger/vhlogger.h>


int main(int argc, char *argv[])
{
    int i = 32;
    VGP_DEBUG("this is one debug msg {}", i);

    auto  logger = vgp::Logger::GetInstance();
    logger->AddLogger("test_logger", "logger.log");
    VGP_INFO_TO("test_logger", "this is one info msg to logger.log");
    return 0;
}
