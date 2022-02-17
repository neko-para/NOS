#pragma once

#include <stdint.h>

struct Gdt {
    enum {
        A_ACCESS = 1 << 0,
        A_READ_WRITE = 1 << 1,
        A_DIRECTION_CONFORMING = 1 << 2,
        A_EXECUTABLE = 1 << 3,
        A_NOT_SYSTEM = 1 << 4,
        A_DPL_0 = 0,
        A_DPL_1 = 1 << 5,
        A_DPL_2 = 1 << 6,
        A_DPL_3 = 3 << 5,
        A_PRESENT = 1 << 7,

        F_LONG = 1 << 1,
        F_SIZE = 1 << 2,
        F_GRANULARITY = 1 << 3
    };
    uint16_t limit_lo;
    uint16_t base_lo;
    uint8_t base_mi;
    uint8_t access;
    uint8_t limit_hi : 4;
    uint8_t flag : 4;
    uint8_t base_hi;

    static void init();

    void setbase(uint32_t base);
    void setlimit(uint32_t limit);

    void set(uint32_t base, uint32_t limit, uint8_t access, uint8_t flag);
};

