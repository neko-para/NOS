#pragma once

#include "../io/isr.h"

struct PtRegs {
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t xds;
    uint32_t xes;
    uint32_t xfs;
    uint32_t xgs;
    uint32_t eax;
    uint32_t eip;
    uint32_t xcs;
    uint32_t eflags;
    uint32_t esp;
    uint32_t xss;
};

extern "C" __attribute__((interrupt)) void isrHandler128(InterruptFrame *frame);
extern "C" uint32_t syscallHandler(PtRegs *regs);
