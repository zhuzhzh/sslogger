#include <ssln/sslogger.h>


int main(int argc, char *argv[])
{
    int i = 32;
    SSLN_DEBUG("this is one debug msg {}", i);

    auto  logger = ssln::Logger::GetInstance();
    logger->AddLogger("test_logger", "logger.log");
    SSLN_INFO_TO("test_logger", "this is one info msg to logger.log");
    return 0;
}
