#include "idt.h"
#include "io.h"
#include "keyboard.h"
#include "term.h"
#include "../util/queue.h"

using KeyboardBuffer = Queue<uint8_t, 256>;
static KeyboardBuffer *buffer;
Semaphore *Keyboard::available = 0;

static char SC_to_char[] =
    "\0\0\0\0001!2@3#4$5%6^7&8*9(0)-_=+\b\b"
    "\t\tqQwWeErRtTyYuUiIoOpP[{]}\n\n"
    "\0\0aAsSdDfFgGhHjJkKlL;:'\"`~\0\0\\|"
    "zZxXcCvVbBnNmM,<.>/?\0\0**"
    "\0\0  \0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    "778899--445566++11223300..\0";

void Keyboard::init() {
    waitReady();
    outb(0x64, 0x60);
    waitReady();
    outb(0x60, 0x47);
    buffer = new KeyboardBuffer;
    available = new Semaphore(1);
    available->lock();
}

void Keyboard::push(uint8_t dat) {
    buffer->push(dat);
    if (available->count()) {
        available->unlock();
    }
}

bool Keyboard::pop(KeyboardMessage &msg) {
    while (!buffer->empty()) {
        uint8_t dat;
        InterruptLock::lock();
        buffer->pop(dat);
        InterruptLock::unlock();
        if (syncPush(dat, &msg)) {
            return true;
        }
    }
    return false;
}

bool Keyboard::syncPush(uint8_t dat, KeyboardMessage *msg) {
    static int state = 0;

    switch (state) {
        case 0:
            if (dat == 0xE0) {
                state = 1;
                return false;
            } else {
                decode(msg, dat);
                return true;
            }
        case 1:
            decode(msg, 0xE0 << 8 | dat);
            state = 0;
            return true;
        default:
            return false;
    }
}

void Keyboard::decode(KeyboardMessage *msg, uint16_t scancode) {
    static uint8_t state = 0;

    msg->code = scancode;
    msg->flag = 0;
    msg->ch = 0;
    msg->key = scancode & 0xFF7F;
    if (scancode & 0x80) {
        msg->flag |= RELEASE;
        scancode -= 0x80;
    }
    if (scancode >> 8) { // 0xE0
        if (scancode == SC_KP_SLASH) {
            msg->ch = '/';
        }
    } else {
        if (scancode < sizeof(SC_to_char) / 2) {
            msg->ch = SC_to_char[scancode * 2 + (state & SHIFT ? 1 : 0)];
        }
    }
    switch (scancode) {
        case SC_LCTRL:
            if (msg->flag & RELEASE) {
                state &= ~CTRL;
            } else {
                state |= CTRL;
            }
            break;
        case SC_LSHIFT:
        case SC_RSHIFT:
            if (msg->flag & RELEASE) {
                state &= ~SHIFT;
            } else {
                state |= SHIFT;
            }
            break;
        case SC_LALT:
            if (msg->flag & RELEASE) {
                state &= ~ALT;
            } else {
                state |= ALT;
            }
            break;
    }
    msg->flag |= state;
}

void Keyboard::waitReady() {
    while (inb(0x64) & 0x2) {
        ;
    }
}

__attribute__((interrupt)) void isrHandler33(InterruptFrame *) {
    Idt::end(1);
    uint8_t ch = inb(0x60);
    Keyboard::push(ch);
}