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
#define __ISR_EXC_NE(n) __attribute__((interrupt)) void isrHandler##n(InterruptFrame *frame)

__ISR_EXC_NE(0);
__ISR_EXC_NE(1);
__ISR_EXC_NE(2);
__ISR_EXC_NE(3);
__ISR_EXC_NE(4);
__ISR_EXC_NE(5);
__ISR_EXC_NE(6);
__ISR_EXC_NE(7);
__ISR_EXC(8);
__ISR_EXC_NE(9);
__ISR_EXC(10);
__ISR_EXC(11);
__ISR_EXC(12);
__ISR_EXC(13);
__ISR_EXC(14);
__ISR_EXC_NE(15);
__ISR_EXC_NE(16);
__ISR_EXC(17);
__ISR_EXC_NE(18);
__ISR_EXC_NE(19);
__ISR_EXC_NE(20);
__ISR_EXC(21);
__ISR_EXC_NE(22);
__ISR_EXC_NE(23);
__ISR_EXC_NE(24);
__ISR_EXC_NE(25);
__ISR_EXC_NE(26);
__ISR_EXC_NE(27);
__ISR_EXC_NE(28);
__ISR_EXC(29);
__ISR_EXC(30);
__ISR_EXC_NE(31);
