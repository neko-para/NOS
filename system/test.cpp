#include "syscall.h"

extern "C" void _start() {
    const char *str = "Hello world from outer elf program!\n";
    nos_print(str);
    nos_exit();
}