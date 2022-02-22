#pragma once

#include "isr.h"
#include "../process/semaphore.h"

struct KeyboardMessage {
    uint16_t code;
    uint16_t key;
    uint8_t flag;
    char ch;
};

class Keyboard {
public:
    enum {
        PRESS = 0,
        RELEASE = 1 << 7,

        SC_ESCAPE = 0x1,
        SC_1,
        SC_2,
        SC_3,
        SC_4,
        SC_5,
        SC_6,
        SC_7,
        SC_8,
        SC_9,
        SC_0,
        SC_MINUS,
        SC_EQUAL,
        SC_BACKSPACE,
        SC_TAB,
        SC_Q,
        SC_W,
        SC_E,
        SC_R,
        SC_T,
        SC_Y,
        SC_U,
        SC_I,
        SC_O,
        SC_P,
        SC_LBRACKET,
        SC_RBRACKET,
        SC_ENTER,
        SC_LCTRL,
        SC_A,
        SC_S,
        SC_D,
        SC_F,
        SC_G,
        SC_H,
        SC_J,
        SC_K,
        SC_L,
        SC_SEMICLONE,
        SC_QUOTE,
        SC_BACKTICK,
        SC_LSHIFT,
        SC_BACKSLASH,
        SC_Z,
        SC_X,
        SC_C,
        SC_V,
        SC_B,
        SC_N,
        SC_M,
        SC_COMMA,
        SC_PERIOD,
        SC_SLASH,
        SC_RSHIFT,
        SC_KP_MULTIPLY,
        SC_LALT,
        SC_SPACE,
        SC_CAPLOCKS,
        SC_F1,
        SC_F2,
        SC_F3,
        SC_F4,
        SC_F5,
        SC_F6,
        SC_F7,
        SC_F8,
        SC_F9,
        SC_F10,
        SC_NUMLOCK,
        SC_SCROLLLOCK,
        SC_KP_7,
        SC_KP_8,
        SC_KP_9,
        SC_KP_MINUS,
        SC_KP_4,
        SC_KP_5,
        SC_KP_6,
        SC_KP_PLUS,
        SC_KP_1,
        SC_KP_2,
        SC_KP_3,
        SC_KP_0,
        SC_KP_DOT,
        SC_F11 = 0x57,
        SC_F12,

        SC_KP_ENTER = 0xE01C,
        SC_RCTRL,
        SC_KP_SLASH = 0xE035,
        SC_RALT = 0xE038,
        SC_HOME = 0xE047,
        SC_UP,
        SC_PAGEUP,
        SC_LEFT = 0xE04B,
        SC_RIGHT = 0xE04D,
        SC_END = 0xE04F,
        SC_DOWN,
        SC_PAGEDOWN,
        SC_INSERT,
        SC_DELETE,

        CTRL = 1,
        SHIFT = 2,
        ALT = 4
    };

    static void init();
    static void push(uint8_t dat);
    static bool pop(KeyboardMessage &msg); // should lock for it
    static bool syncPush(uint8_t dat, KeyboardMessage *msg);
    static void decode(KeyboardMessage *msg, uint16_t scancode);

    static void waitReady();
    static Semaphore *available;
};

__attribute__((interrupt)) void isrHandler33(InterruptFrame *frame);