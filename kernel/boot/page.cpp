#include "page.h"
#include "../util/memory.h"
#include "../util/new.h"

Page *flatPage;

void Page::init() {
    flatPage = new Page;
    uint32_t *entry = reinterpret_cast<uint32_t *>(Frame::alloc());
    for (uint32_t i = 0; i < 1024; i++) {
        entry[i] = (i << 12) | Page::NON_SUPERVISOR | Page::READWRITE | Page::PRESENT;
    }
    flatPage->set(0, entry, Page::NON_SUPERVISOR | Page::READWRITE | Page::PRESENT);
    flatPage->load();
    enable();
}

void Page::enable() {
    asm volatile ( "movl %cr0, %eax; or $0x80000000, %eax; movl %eax, %cr0;" );
}

Page::Page() {
    pageDirectory = reinterpret_cast<uint32_t *>(Frame::alloc());
    for (uint32_t i = 0; i < 1024; i++) {
        pageDirectory[i] = Page::NON_SUPERVISOR | Page::READWRITE;
    }
}

void Page::set(uint32_t index, uint32_t *pageEntry, uint8_t attrib) {
    pageDirectory[index] = (reinterpret_cast<uint32_t>(pageEntry) & (~0x3FF)) | (attrib & 0x3FF);
}

void Page::load() {
    asm volatile ( "movl %0, %%eax; movl %%eax, %%cr3;" : "=m"(pageDirectory) :: "%eax" );
}