#pragma once

#include <stdint.h>

struct MBREntry {
    enum {
        A_ACTIVE = 1 << 7
    };

    uint8_t attrib;
    uint8_t chs_start[3];
    uint8_t type;
    uint8_t chs_end[3];
    uint32_t lba_start;
    uint32_t count;

    bool isValid();
    bool isActive() {
        return attrib & A_ACTIVE;
    }
};

struct MBR {
    uint8_t boot[440];
    uint8_t id[4];
    uint16_t readonly;
    MBREntry entry[4];
    uint16_t signature;

    void load(uint32_t drive);
} __attribute__((packed));
