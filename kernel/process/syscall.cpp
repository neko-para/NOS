#include "syscall.h"
#include "task.h"
#include "../io/term.h"

extern "C" uint32_t syscallHandler(PtRegs *regs) {
    switch (regs->eax) {
    case 1: // exit
        Task::exit();
        return 0;
    case 2: // fork
        return Task::fork(reinterpret_cast<uint32_t>(&regs->ebx));
    case 4: // write
    {
        const char *buf = reinterpret_cast<const char *>(regs->ebx);
        for (uint32_t i = 0; i < regs->ecx; i++) {
            term() << buf[i];
        }
        return 0;
    }
    }
}
