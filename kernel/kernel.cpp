#include "boot/bootinfo.h"
#include "boot/gdt.h"
#include "boot/page.h"
#include "filesystem/mbr.h"
#include "filesystem/ext2.h"
#include "filesystem/deviceFS.h"
#include "filesystem/procFS.h"
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
#include "vfs/vfs.h"

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

void mainTask() {
    Task::inited = true;

    VFS::init();

    MBR mbr;
    mbr.load(0);
    EXT2 *ext2 = new EXT2(0, mbr.entry[0].lba_start, mbr.entry[0].count);
    VFS::mount("/", ext2);
    VFS::mount("/dev", new DeviceFS());
    VFS::mount("/proc", new ProcFS());

    currentTask->file->pushBack(VFS::lookup("/dev/tty").open("/dev/tty", 0)); // stdin
    currentTask->file->pushBack(VFS::lookup("/dev/tty").open("/dev/tty", 0)); // stdout

    auto f = VFS::lookup("/bin/test");
    Stat st;
    f.getRegularFile()->stat(&st);
    void *prog = Memory::alloc(st.size);
    auto fd = f.open("/bin/test", 0);
    fd->read(prog, st.size);
    fd->close();
    delete fd;
    ELF *elf = new ELF(prog);
    Task::loadELF(elf);

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