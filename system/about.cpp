#include <unistd.h>

extern "C" void _start() {
    write(1, "NOS: nekosu's operating system v1", 33);
    _exit(0);
}