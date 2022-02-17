#pragma once

#include <stdint.h>

struct InterruptFrame { 
    uint32_t ip; 
    uint32_t cs; 
    uint32_t flags;
    uint32_t sp;
    uint32_t ss;
};

#define __ISR_EXC(n) __attribute__((interrupt)) void isrHandler##n(InterruptFrame *frame, uint32_t error)

__ISR_EXC(0);
__ISR_EXC(1);
__ISR_EXC(2);
__ISR_EXC(3);
__ISR_EXC(4);
__ISR_EXC(5);
__ISR_EXC(6);
__ISR_EXC(7);
__ISR_EXC(8);
__ISR_EXC(9);
__ISR_EXC(10);
__ISR_EXC(11);
__ISR_EXC(12);
__ISR_EXC(13);
__ISR_EXC(14);
__ISR_EXC(15);
__ISR_EXC(16);
__ISR_EXC(17);
__ISR_EXC(18);
__ISR_EXC(19);
__ISR_EXC(20);
__ISR_EXC(21);
__ISR_EXC(22);
__ISR_EXC(23);
__ISR_EXC(24);
__ISR_EXC(25);
__ISR_EXC(26);
__ISR_EXC(27);
__ISR_EXC(28);
__ISR_EXC(29);
__ISR_EXC(30);
__ISR_EXC(31);
