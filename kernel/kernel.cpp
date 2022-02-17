#include "boot/bootinfo.h"
#include "boot/gdt.h"
#include "io/term.h"
#include "util/debug.h"
#include "util/memory.h"

void prepareMemory(BootInfo *info) {
    for (uint32_t i = 0; i < info->mmap_length / sizeof(MMapEntry); i++) {
        MMapEntry *me = info->mmap_addr + i;
        if (me->addr_hi || me->len_hi) {
            continue;
        }
        if (me->type == 1 && me->addr_lo > 0) { // skip first
            if (me->addr_lo == 0x100000) {
                uint32_t size = (uint32_t)&image_size;
                me->addr_lo = (size + 0xFFF) & (~0xFFF);
            }
            Memory::add(me->addr_lo, me->len_lo);
            debug() << hex << me->addr_lo << '-' << (me->addr_lo + me->len_lo) << endl;
        }
    }
    debug() << dec;
}

extern "C" void kernel_main(BootInfo *info) {
    Gdt::init();

    QemuDebug::init();

    Memory::init();
    prepareMemory(info);

    Term::init();

    term() << "Gdt set! Still printing well." << endl;
    debug() << "From debug()!" << endl;
}