#include "ssln/sslogger.h"

using ssln::g_logger;

int main(int argc, char *argv[])
{
    int i = 32;
    SSLN_DEBUG("this is one debug msg {}", i);

    SSLN_INFOF("This is one Info msg in file {}", i);
    return 0;
}
