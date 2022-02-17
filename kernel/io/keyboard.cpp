#include "idt.h"
#include "io.h"
#include "keyboard.h"
#include "term.h"

void Keyboard::init() {
    waitReady();
    outb(0x64, 0x60);
    waitReady();
    outb(0x60, 0x47);
}

void Keyboard::waitReady() {
    while (inb(0x64) & 0x2) {
        ;
    }
}

__attribute__((interrupt)) void isrHandler33(InterruptFrame *frame) {
    Idt::end(1);
    uint8_t ch = inb(0x60);
    // term() << ch << endl;
}