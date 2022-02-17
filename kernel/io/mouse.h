#pragma once

#include "isr.h"

class Mouse {
public:
    static void init();
};

__attribute__((interrupt)) void isrHandler44(InterruptFrame *frame);
