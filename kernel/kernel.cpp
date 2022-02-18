#include "boot/bootinfo.h"
#include "boot/gdt.h"
#include "boot/page.h"
#include "filesystem/mbr.h"
#include "io/idt.h"
#include "io/io.h"
#include "io/keyboard.h"
#include "io/mouse.h"
#include "io/term.h"
#include "io/timer.h"
#include "process/task.h"
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
                // acually always one hole
                // use &image_size ~ 16M as kernel heap, 16M ~ 1G as pages.
                uint32_t size = (uint32_t)&image_size;
                me->addr_lo = (size + 0xFFF) & (~0xFFF);
                Memory::add(me->addr_lo, 0x1000000 - me->addr_lo);
                Frame::init(0x1000000, (me->len_lo - 0xF00000) >> 12);
                Page::init(me->len_lo + 0x100000);
            }
        }
    }
}

void userTask() {
    const char *str = "Hello world from Ring3!\n";
    asm volatile ( "movl %0, %%esi; movl $2, %%eax; int $0x80;" : "=m"(str) );
    asm volatile ( "movl $0, %eax; int $0x80;" );
}

void subTask() {
    Task::unlock();
    term() << "subtask created" << endl;
    Task::enterRing3(userTask);
}

void mainTask() {
    term() << "main task created" << endl;
/*
    Page *userFlatPage = new Page;
    uint32_t count = 1 << 8;
    for (uint32_t i = 0; i < count; i++) {
        uint32_t *entry = reinterpret_cast<uint32_t *>(Frame::alloc());
        for (uint32_t j = 0; j < 1024; j++) {
            entry[j] = (i << 22) | (j << 12) | Page::NON_SUPERVISOR | Page::READWRITE | Page::PRESENT;
        }
        userFlatPage->set(i, entry, Page::NON_SUPERVISOR | Page::READWRITE | Page::PRESENT);
    }

    Task::create(subTask, userFlatPage->cr3());
*/
    MBR mbr;
    mbr.load(0);
    term() << "Signature is " << hex << mbr.signature << dec << endl;
    for (int i = 0; i < 4; i++) {
        term() << "Partition " << i << " attrib " << hex << mbr.entry[i].attrib << dec;
        if (mbr.entry[i].isValid()) {
            term() << " valid";
            if (mbr.entry[i].isActive()) {
                term() << " active";
            }
            term() << "; starting from " << mbr.entry[i].lba_start << " with " << mbr.entry[i].count << " sectors";
        }
        term() << endl;
    }
    while (true) {
        hlt();
        if (Task::lockSchedule()) {
            term() << "back to main" << endl;
        }
    }
}

extern "C" void kernel_main(BootInfo *info) {
    Gdt::init();
    Idt::init();

    QemuDebug::init();
    Term::init();

    Memory::init();
    prepareMemory(info);

    Keyboard::init();
    Mouse::init();
    Timer::set(100);

    Idt::unmask(0);
    Idt::unmask(1);
    Idt::unmask(2);
    Idt::unmask(12);

    sti();

    term() << "Everything alright!" << endl;

    Task::init(mainTask);

    term() << "Fallout!" << endl;

    cli();
    while (true) {
        hlt();
    }
}