#include "io.h"

static uint32_t lockCount = 0;

void intLock() {
    cli();
    lockCount++;
}

void intUnlock() {
    if (--lockCount == 0) {
        sti();
    }
}