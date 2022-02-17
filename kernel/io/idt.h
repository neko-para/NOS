#pragma once

#include <stdint.h>

struct IdtEntry {
    enum {
        A_PRESENT = 1 << 7,
        A_DPL_0 = 0,
        A_DPL_1 = 1 << 5,
        A_DPL_2 = 1 << 6,
        A_DPL_3 = 3 << 5,
        A_GATE_TASK = 0x5,
        A_GATE_INT_16 = 0x6,
        A_GATE_TRAP_16 = 0x7,
        A_GATE_INT_32 = 0xE,
        A_GATE_TRAP_32 = 0xF
    };

    uint16_t isr_low;
    uint16_t kernel_cs;
    uint8_t _;
    uint8_t attribute;
    uint16_t isr_high;

    void set(uint32_t isr, uint8_t attr);
};

struct IDTR {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

class Idt {
public:
    static void init();

    static void mask(uint8_t irq);
    static void unmask(uint8_t irq);
    static void end(uint8_t irq);
};
