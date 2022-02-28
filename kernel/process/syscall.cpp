#include <sys/syscall.h>
#include <sys/stat.h>
#include "syscall.h"
#include "task.h"
#include "../vfs/vfs.h"

extern "C" int32_t syscallHandler(PtRegs *regs) {
    switch (regs->eax) {
    case SYS_exit: // exit
        Task::exit();
        return 0;
    case SYS_fork: // fork
        return Task::fork(reinterpret_cast<uint32_t>(&regs->ebx));
    case SYS_read: // read
    {
        int32_t fd = regs->ebx;
        void *buf = reinterpret_cast<void *>(regs->ecx);
        const auto &node = (*currentTask->file)[fd];
        return node->file->read(buf, regs->edx);
    }
    case SYS_write: // write
    {
        int32_t fd = regs->ebx;
        auto node = (*currentTask->file)[fd];
        if (!node) {
            return -1;
        }
        const void *buf = reinterpret_cast<const void *>(regs->ecx);
        return node->file->write(buf, regs->edx);
    }
    case SYS_open: // open
    {
        const char *path = reinterpret_cast<const char *>(regs->ebx);
        int32_t flag = regs->ecx;
        VFS::FilePtr f = VFS::lookup(path);
        currentTask->file->pushBack(f.open(path, flag));
        return currentTask->file->size() - 1;
    }
    case SYS_close: // close
    {
        int32_t fd = regs->ebx;
        auto node = (*currentTask->file)[fd];
        if (!node) {
            return -1;
        }
        (*currentTask->file)[fd] = 0;
        delete node;
        return 0;
    }
    case SYS_execve: // execve
    {
        const char *path = reinterpret_cast<const char *>(regs->ebx);
        auto f = VFS::lookup(path);
        if (!f.get()) {
            return -1;
        }
        struct stat st;
        f.getRegularFile()->stat(&st);
        void *prog = Memory::alloc(st.st_size);
        auto fd = f.open(path, 0);
        fd->read(prog, st.st_size);
        fd->close();
        delete fd;
        ELF *elf = new ELF(prog);
        Task::replaceViaELF(elf);
        return 0; // dead code
    }
    case SYS_stat: // stat
    {
        const char *path = reinterpret_cast<const char *>(regs->ebx);
        struct stat *buf = reinterpret_cast<struct stat *>(regs->ecx);
        return VFS::lookup(path).get()->stat(buf);
    }
    case SYS_lseek: // lseek
    {
        int32_t fd = regs->ebx;
        auto node = (*currentTask->file)[fd];
        if (!node) {
            return -1;
        }
        return node->seek(regs->ecx, regs->edx);
    }
    case SYS_getpid: // getpid
        return currentTask->tid;
    }
    return -1;
}
