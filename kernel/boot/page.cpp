#include "page.h"
#include "../util/memory.h"
#include "../util/new.h"
#include "../util/string.h"

Page *flatPage;

void Page::init(uint32_t maxiaddr) {
    flatPage = new Page;
    uint32_t count = (maxiaddr + (1 << 22) - 1) >> 22;
    for (uint32_t i = 0; i < count; i++) {
        uint32_t *entry = reinterpret_cast<uint32_t *>(Frame::alloc());
        for (uint32_t j = 0; j < 1024; j++) {
            entry[j] = (i << 22) | (j << 12) | READWRITE | PRESENT;
        }
        flatPage->set(i, entry, READWRITE | PRESENT);
    }
    flatPage->load();
    enable();
}

void Page::enable() {
    asm volatile ( "movl %cr0, %eax; orl $0x80000000, %eax; movl %eax, %cr0;" );
}

Page::Page(uint32_t *preset) {
    if (preset) {
        pageDirectory = preset;
    } else {
        pageDirectory = reinterpret_cast<uint32_t *>(Frame::alloc());
        for (uint32_t i = 0; i < 1024; i++) {
            pageDirectory[i] = 0;
        }
    }
}

void Page::set(uint32_t index, uint32_t *pageEntry, uint8_t attrib) {
    pageDirectory[index] = (reinterpret_cast<uint32_t>(pageEntry) & (~0xFFF)) | (attrib & 0xFFF);
}

void Page::load() {
    asm volatile ( "movl %0, %%eax; movl %%eax, %%cr3;" : "=m"(pageDirectory) :: "%eax" );
}

bool Page::isSet(uint32_t virAddr) {
    uint32_t virPre = virAddr >> 22;
    uint32_t virMid = (virAddr >> 12) & 0x3FF;
    if (!pageDirectory[virPre]) {
        return false;
    }
    uint32_t *entry = reinterpret_cast<uint32_t *>(pageDirectory[virPre] & ~0xFFF);
    return entry[virMid] != 0;
}

void Page::autoSet(uint32_t phyAddr, uint32_t virAddr, uint8_t attrib) {
    uint32_t phyPre = phyAddr & ~0xFFF;
    uint32_t virPre = virAddr >> 22;
    uint32_t virMid = (virAddr >> 12) & 0x3FF;
    if (pageDirectory[virPre] == 0) {
        uint32_t *entry = reinterpret_cast<uint32_t *>(Frame::alloc());
        memset(entry, 0, 0x1000);
        entry[virMid] = phyPre | attrib;
        set(virPre, entry, attrib);
    } else {
        uint32_t *entry = reinterpret_cast<uint32_t *>(pageDirectory[virPre] & ~0xFFF);
        entry[virMid] = phyPre | attrib;
        pageDirectory[virPre] |= attrib;

    }
}

void Page::autoSet(uint32_t phyAddr, uint32_t virAddr, uint32_t size, uint8_t attrib) {
    size = (size + 0xFFF) >> 12;
    while (size--) {
        autoSet(phyAddr, virAddr, attrib);
        phyAddr += 0x1000;
        virAddr += 0x1000;
    }
}

void Page::free() {
    for (int i = 0; i < 1024; i++) {
        if (pageDirectory[i]) {
            Frame::free(reinterpret_cast<void *>(pageDirectory[i] & ~0xFFF));
        }
    }
    Frame::free(pageDirectory);
}
