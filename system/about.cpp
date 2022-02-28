#include <unistd.h>

extern "C" void _start() {
    write(1, "NOS: nekosu's operating system v1\n", 34);
    _exit(1);
}