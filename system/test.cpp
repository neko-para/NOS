#include "syscall.h"

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
    char *test = reinterpret_cast<char *>(1 << 28);
    int32_t p = fork();
    const char *str = "PID: \n";
    strcpy(test, str);
    test[4] = p + '0';
    write(1, test, strlen(test));
    exit(0);
}