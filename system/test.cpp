#include <unistd.h>

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
        execve("/bin/about", 0, 0);
    }
    _exit(0);
}