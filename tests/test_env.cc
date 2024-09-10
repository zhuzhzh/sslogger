#include <vhlogger/vhlogger.h>


int main(int argc, char *argv[])
{
    int i = 32;
    VGP_DEBUG("this is one debug msg {}", i);

    VGP_INFOF("This is one Info msg in file {}", i);
    return 0;
}
