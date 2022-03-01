#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include "syscall.h"
#include "task.h"
#include "../vfs/vfs.h"

extern "C" int32_t syscallHandler(PtRegs *regs) {
    switch (regs->eax) {
    case SYS_exit:
        Task::exit(regs->ebx);
        return 0;
    case SYS_fork:
        return Task::fork(reinterpret_cast<uint32_t>(&regs->ebx));
    case SYS_read:
    {
        int32_t fd = regs->ebx;
        void *buf = reinterpret_cast<void *>(regs->ecx);
        const auto &node = (*currentTask->file)[fd];
        return node->file->read(buf, regs->edx);
    }
    case SYS_write:
    {
        int32_t fd = regs->ebx;
        auto node = (*currentTask->file)[fd];
        if (!node) {
            return -1;
        }
        const void *buf = reinterpret_cast<const void *>(regs->ecx);
        return node->file->write(buf, regs->edx);
    }
    case SYS_open:
    {
        const char *path = reinterpret_cast<const char *>(regs->ebx);
        int32_t flag = regs->ecx;
        VFS::FilePtr f = VFS::lookup(path);
        currentTask->file->pushBack(f.open(path, flag));
        return currentTask->file->size() - 1;
    }
    case SYS_close:
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
    case SYS_waitpid:
    {
        LockGuard<PostponeScheduleLock> _;
        pid_t pid = regs->ebx;
        int *wstatus = reinterpret_cast<int *>(regs->ecx);
        int options = regs->edx;
        if (pid == 0) { // no process group supported, fallback
            pid = -1;
        } else if (pid < -1) {
            return -1;
        }
        if (pid > 0) {
            if (pid >= MaxTask) {
                return -1;
            }
            TaskControlBlock *task = tasks[pid];
            if (task && task->parent != currentTask->tid) {
                return -1;
            }
            if (task->getRunningState() == TaskControlBlock::TERMINATED) {
                int32_t res = Task::clean(task->tid);
                if (wstatus) {
                    *wstatus = res;
                }
                return 0;
            } else {
                if (options & WNOHANG) {
                    if (wstatus) {
                        *wstatus = 0;
                    }
                    return 0;
                } else {
                    currentTask->param = pid;
                    PostponeScheduleLock::unlock();
                    Task::block(TaskControlBlock::WAITCHILD);
                    PostponeScheduleLock::lock();
                    if (task->getRunningState() == TaskControlBlock::TERMINATED) {
                        int32_t res = Task::clean(task->tid);
                        if (wstatus) {
                            *wstatus = res;
                        }
                        return 0;
                    } else {
                        return -1;
                    }
                }
            }
        } else {
            for (uint32_t i = 0; i < currentTask->child->size(); i++) {
                int32_t id = (*currentTask->child)[i];
                TaskControlBlock *task = tasks[id];
                if (task->getRunningState() == TaskControlBlock::TERMINATED) {
                    int32_t res = Task::clean(task->tid);
                    if (wstatus) {
                        *wstatus = res;
                    }
                    return 0;
                }
            }
            if (options & WNOHANG) {
                if (wstatus) {
                    *wstatus = 0;
                }
                return 0;
            } else {
                currentTask->param = 0;
                Task::block(TaskControlBlock::WAITCHILD);
                TaskControlBlock *task = tasks[currentTask->param];
                if (task->getRunningState() == TaskControlBlock::TERMINATED) {
                    int32_t res = Task::clean(task->tid);
                    if (wstatus) {
                        *wstatus = res;
                    }
                    return 0;
                } else {
                    return -1;
                }
            }
        }
    }
    case SYS_execve:
    {
        const char *path = reinterpret_cast<const char *>(regs->ebx);
        char **argv = reinterpret_cast<char **>(regs->ecx);
        char **envp = reinterpret_cast<char **>(regs->edx);
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
        Task::replaceViaELF(elf, argv, envp);
        return 0;
    }
    case SYS_stat:
    {
        const char *path = reinterpret_cast<const char *>(regs->ebx);
        struct stat *buf = reinterpret_cast<struct stat *>(regs->ecx);
        return VFS::lookup(path).get()->stat(buf);
    }
    case SYS_lseek:
    {
        int32_t fd = regs->ebx;
        auto node = (*currentTask->file)[fd];
        if (!node) {
            return -1;
        }
        return node->seek(regs->ecx, regs->edx);
    }
    case SYS_getpid:
        return currentTask->tid;
    case SYS_getdents:
    {
        int32_t fd = regs->ebx;
        auto node = (*currentTask->file)[fd];
        if (!node) {
            return -1;
        }
        if (!node->file.getDirectory()) {
            return -1;
        }
        nos_dirent *dirp = reinterpret_cast<nos_dirent *>(regs->ecx);
        uint32_t count = regs->edx;
        return node->getdents(dirp, count);
    }
    }
    return -1;
}
