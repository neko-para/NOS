#pragma once

#include <stdint.h>
#include "../process/task_control_block.h"

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

    static void init(uint32_t maxiaddr);
    static void enable();

    uint32_t cr3() const {
        return reinterpret_cast<uint32_t>(pageDirectory);
    }

    Page(uint32_t *preset = 0);
    Page(uint32_t cr3) : Page(reinterpret_cast<uint32_t *>(cr3)) {}

    uint32_t clone(TaskControlBlock *target);
    void checkPresent();

    void set(uint32_t index, uint32_t *pageEntry, uint8_t attrib);
    void load();

    bool isSet(uint32_t virAddr);
    void autoSet(uint32_t phyAddr, uint32_t virAddr, uint8_t attrib);
    void autoSet(uint32_t phyAddr, uint32_t virAddr, uint32_t size, uint8_t attrib);

    void free(); // free tables; not the pages

    static void *mount(uint32_t phyAddr); // mount to tempPageAddr
    static void *mount(void *phyAddr) {
        return mount(reinterpret_cast<uint32_t>(phyAddr));
    }

private:
    uint32_t *pageDirectory;
};

constexpr uint32_t tempPageAddr = (1 << 27) - 0x1000;

extern Page *flatPage;