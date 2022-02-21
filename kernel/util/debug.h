#pragma once

#include "stream.h"
#include "../io/term.h"

#define BOCHS_MAGIC_BREAKPOINT asm volatile ("xchgw %bx, %bx")

#define FATAL do { term() << "[FATAL ERROR: " << __FILE__ << ' ' << __LINE__ << ']'; asm volatile ("cli; hlt"); } while (0)

class QemuDebug : public Stream {
public:
    virtual void put(char ch) override;

    static void init();
};

QemuDebug &debug();