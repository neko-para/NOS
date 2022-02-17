#pragma once

#include <stdint.h>
#include "isr.h"

class Keyboard {
public:
    static void init();

    static void waitReady();
};

__attribute__((interrupt)) void isrHandler33(InterruptFrame *frame);