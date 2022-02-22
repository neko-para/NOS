#include "syscall.h"

extern "C" {

void exit(int32_t ret) {
    asm volatile ( "movl %0, %%ebx; movl $1, %%eax; int $0x80;" : "=m"(ret) );
}

int32_t fork() {
    int32_t res;
    asm volatile ( "movl $2, %%eax; int $0x80; movl %%eax, %0; " : "=m"(res) );
    return res;
}


void write(int32_t fd, void *buf, uint32_t len) {
    asm volatile ( "movl %0, %%ebx; movl %1, %%ecx; movl $4, %%eax; int $0x80;" : "=m"(buf), "=m"(len) );
}

}