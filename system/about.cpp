#include <unistd.h>

extern "C" int _main() {
    write(1, "NOS: nekosu's operating system v1\n", 34);
    return 1;
}