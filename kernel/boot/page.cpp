#include "page.h"
#include "../util/memory.h"
#include "../util/new.h"

Page *flatPage;

void Page::init(uint32_t maxiaddr) {
    flatPage = new Page;
    uint32_t count = (maxiaddr + (1 << 22) - 1) >> 22;
    for (uint32_t i = 0; i < count; i++) {
        uint32_t *entry = reinterpret_cast<uint32_t *>(Frame::alloc());
        for (uint32_t j = 0; j < 1024; j++) {
            entry[j] = (i << 22) | (j << 12) | Page::READWRITE | Page::PRESENT;
        }
        flatPage->set(i, entry, Page::READWRITE | Page::PRESENT);
    }
    flatPage->load();
    enable();
}

void Page::enable() {
    asm volatile ( "movl %cr0, %eax; orl $0x80000000, %eax; movl %eax, %cr0;" );
}

Page::Page() {
    pageDirectory = reinterpret_cast<uint32_t *>(Frame::alloc());
    for (uint32_t i = 0; i < 1024; i++) {
        pageDirectory[i] = Page::READWRITE;
    }
}

void Page::set(uint32_t index, uint32_t *pageEntry, uint8_t attrib) {
    pageDirectory[index] = (reinterpret_cast<uint32_t>(pageEntry) & (~0x3FF)) | (attrib & 0x3FF);
}

void Page::load() {
    asm volatile ( "movl %0, %%eax; movl %%eax, %%cr3;" : "=m"(pageDirectory) :: "%eax" );
}