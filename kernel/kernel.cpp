#include "boot/bootinfo.h"
#include "boot/gdt.h"
#include "boot/page.h"
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

void otherTask() {
    Task::unlock();
    term() << "other task created" << endl;
    int n = 5;
    while (n--) {
        term() << "other count down " << n << endl;
        Task::lock();
        Task::schedule();
        Task::unlock();
    }
    term() << "other task destroyed" << endl;
    Task::lock();
    Task::exit();
}

void mainTask() {
    term() << "main task created" << endl;
    Task::create(otherTask);
    while (true) {
        hlt();
        Task::lock();
        if (Task::schedule()) {
            term() << "back to main" << endl;
        }
        Task::unlock();
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