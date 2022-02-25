#include "../kernel/export/syscall.h"

void strcpy(char *dst, const char *src) {
    while (*src) {
        *dst++ = *src++;
    }
    *dst = 0;
}

uint32_t strlen(const char *src) {
    uint32_t l = 0;
    while (*src++) {
        l++;
    }
    return l;
}

extern "C" void _start() {
    write(1, "From parent!\n", 13);
    int32_t p = fork();
    if (p) { // parent
        write(1, "parent exit!\n", 13);
    } else {
        write(1, "From child!\n", 12);
        uint8_t ch[] = "you enter: ?\n";
        read(0, ch + 11, 1);
        write(1, ch, 13);
        write(1, "child exit!\n", 12);
    }
    exit(0);
}