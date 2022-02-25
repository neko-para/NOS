#include "syscall.h"

static int32_t _syscall(uint32_t eax, uint32_t ebx = 0, uint32_t ecx = 0, uint32_t edx = 0) {
    int32_t res;
    asm volatile ( "movl %0, %%ebx; movl %1, %%ecx; movl %2, %%edx; movl %3, %%eax; int $0x80; movl %%eax, %4; " : "=m"(ebx), "=m"(ecx), "=m"(edx), "=m"(eax), "=m"(res) );
    return res;
}

template <typename Type>
inline uint32_t conv(Type t) {
    return static_cast<uint32_t>(t);
}

template <typename Type>
inline uint32_t conv(Type *t) {
    return reinterpret_cast<uint32_t>(t);
}

template <typename TB, typename TC, typename TD>
static int32_t syscall(uint32_t eax, TB ebx, TC ecx, TD edx) {
    return _syscall(eax, conv(ebx), conv(ecx), conv(edx));
}

template <typename TB, typename TC>
static int32_t syscall(uint32_t eax, TB ebx, TC ecx) {
    return syscall(eax, ebx, ecx, 0);
}

template <typename TB>
static int32_t syscall(uint32_t eax, TB ebx) {
    return syscall(eax, ebx, 0, 0);
}

static int32_t syscall(uint32_t eax) {
    return syscall(eax, 0, 0, 0);
}

extern "C" {

void exit(int32_t ret) {
    syscall(1, ret);
}

int32_t fork() {
    return syscall(2);
}

int32_t read(int32_t fd, void *buf, uint32_t len) {
    return syscall(3, fd, buf, len);
}

int32_t write(int32_t fd, const void *buf, uint32_t len) {
    return syscall(4, fd, buf, len);
}

int32_t stat(const char *path, struct Stat *buf) {
    return syscall(18, path, buf);
}


}