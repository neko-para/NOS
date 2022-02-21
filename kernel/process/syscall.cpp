#include "syscall.h"
#include "task.h"
#include "../io/term.h"

extern "C" void syscallHandler(PtRegs *regs) {
    switch (regs->eax) {
    case 0: // exit
        Task::exit();
        break;
    case 2: // write
        term() << reinterpret_cast<const char *>(regs->esi);
        break; 
    }
}
