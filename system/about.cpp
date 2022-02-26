#include <nos.h>

extern "C" void _start() {
    write(1, "NOS: nekosu's operating system v1", 33);
    exit(0);
}