#include "idt.h"
#include "io.h"
#include "term.h"
#include "timer.h"

void Timer::set(uint32_t hz) {
    cli();
    uint16_t h = DEFAULT_HZ / hz;
    outb(0x43, 0x34);
    outb(0x40, h & 0xFF);
    outb(0x40, h >> 8);
}

__attribute__((interrupt)) void isrHandler32(InterruptFrame *frame) {
    Idt::end(0);
    // term() << '.';
}