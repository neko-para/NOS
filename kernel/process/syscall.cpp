#include "syscall.h"
#include "task.h"
#include "../export/syscall.h"
#include "../vfs/vfs.h"

extern "C" int32_t syscallHandler(PtRegs *regs) {
    switch (regs->eax) {
    case 1: // exit
        Task::exit();
        return 0;
    case 2: // fork
        return Task::fork(reinterpret_cast<uint32_t>(&regs->ebx));
    case 3: // read
    {
        int32_t fd = regs->ebx;
        void *buf = reinterpret_cast<void *>(regs->ecx);
        const auto &node = (*currentTask->file)[fd];
        return node->file->read(buf, regs->edx);
    }
    case 4: // write
    {
        int32_t fd = regs->ebx;
        const void *buf = reinterpret_cast<const void *>(regs->ecx);
        const auto &node = (*currentTask->file)[fd];
        return node->file->write(buf, regs->edx);
    }
    case 5: // open
    {
        const char *path = reinterpret_cast<const char *>(regs->ebx);
        int32_t flag = regs->ecx;
        VFS::FilePtr f = VFS::lookup(path);
        currentTask->file->pushBack(f.open(path, flag));
        return currentTask->file->size() - 1;
    }
    case 18: // stat
        const char *path = reinterpret_cast<const char *>(regs->ebx);
        Stat *buf = reinterpret_cast<Stat *>(regs->ecx);
        return VFS::lookup(path).get()->stat(buf);
    }
}
