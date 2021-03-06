#pragma once

#include "isr.h"

class Timer {
    enum {
        DEFAULT_HZ = 1193182
    };
public:
    static void set(uint32_t hz);

    static uint64_t msSinceBoot;
};

__attribute__((interrupt)) void isrHandler32(InterruptFrame *frame);
