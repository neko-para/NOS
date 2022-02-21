#include "io.h"

template<> uint32_t InterruptLock::count = 0;

void _InterruptLock::lock() {
    cli();
}

void _InterruptLock::unlock() {
    sti();
}