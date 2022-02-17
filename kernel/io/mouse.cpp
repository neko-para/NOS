#include "idt.h"
#include "io.h"
#include "keyboard.h"
#include "mouse.h"
#include "term.h"

void Mouse::init() {
    Keyboard::waitReady();
    outb(0x64, 0xd4);
    Keyboard::waitReady();
    outb(0x60, 0xf4);
}

__attribute__((interrupt)) void isrHandler44(InterruptFrame *frame) {
    Idt::end(12);
    uint8_t ch = inb(0x60);
    // term() << ch << endl;
}