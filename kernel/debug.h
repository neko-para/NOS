#pragma once

#include "stream.h"

#define BOCHS_MAGIC_BREAKPOINT asm volatile ("xchgw %bx, %bx")

class QemuDebug : public Stream {
public:
    virtual void put(char ch) override;

    static void init();
};

QemuDebug &debug();