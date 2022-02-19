#include "syscall.h"

extern "C" {

void nos_exit() {
    asm volatile ( "movl $0, %eax; int $0x80;" );
}

void nos_print(const char *str) {
    asm volatile ( "movl %0, %%esi; movl $2, %%eax; int $0x80;" : "=m"(str) );
}

}