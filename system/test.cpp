#include "syscall.h"

void strcpy(char *dst, const char *src) {
    while (*src) {
        *dst++ = *src++;
    }
}

extern "C" void _start() {
    char *test = reinterpret_cast<char *>(1 << 28);
    const char *str = "Hello world from outer elf program!\n";
    strcpy(test, str);
    nos_print(test);
    nos_exit();
}