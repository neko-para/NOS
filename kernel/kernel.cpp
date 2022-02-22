#include "boot/bootinfo.h"
#include "boot/gdt.h"
#include "boot/page.h"
#include "filesystem/mbr.h"
#include "filesystem/ext2.h"
#include "io/idt.h"
#include "io/io.h"
#include "io/keyboard.h"
#include "io/mouse.h"
#include "io/term.h"
#include "io/timer.h"
#include "process/elf.h"
#include "process/task.h"
#include "process/semaphore.h"
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

void request(uint32_t init) {
    Task::unlock();

    Task::sleep(init);

    while (true) {
        term() << currentTask->tid;
        Task::sleep(1000);
    }

    Task::exit();
}

void rotate(uint32_t ) {
    Task::unlock();
    char buf[] = "< ";
    int i = 0;

    while (true) {
        term() << buf[i] << '\b';
        i ^= 1;
        Task::sleep(500);
    }
}

void handleInput(uint32_t ) {
    Task::unlock();

    while (true) {
        Keyboard::available->lock();
        KeyboardMessage msg;
        if (Keyboard::pop(msg)) {
            if (!(msg.flag & Keyboard::RELEASE) && msg.ch) {
                term() << msg.ch;
            }
        }
    }
}

void mainTask() {
    Task::inited = true;

    term().disable_cursor();
    Task::create(rotate);
    Task::create(handleInput, 0, 0, 11);

    Task::exit();
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
    Timer::set(1000);

    Idt::unmask(0);
    Idt::unmask(1);
    Idt::unmask(2);
    Idt::unmask(12);

    sti();

    Task::init(mainTask);

    term() << "Fallout!" << endl;

    cli();
    while (true) {
        hlt();
    }
}