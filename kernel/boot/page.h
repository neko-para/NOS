#pragma once

#include <stdint.h>

class Page {
public:
    enum {
        PRESENT = 1,
        READWRITE = 2,
        NON_SUPERVISOR = 4,
        PWT = 8,
        PCD = 16,
        ACCESSED = 32,
        DIRTY = 64,
    };

    static void init();
    static void enable();

    Page();

    void set(uint32_t index, uint32_t *pageEntry, uint8_t attrib);
    void load();

private:
    uint32_t *pageDirectory;
};

extern Page *flatPage;