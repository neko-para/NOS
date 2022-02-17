#pragma once

#include "isr.h"

class Keyboard {
public:
    static void init();

    static void waitReady();
};

__attribute__((interrupt)) void isrHandler33(InterruptFrame *frame);