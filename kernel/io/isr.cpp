#include "isr.h"
#include "../io/term.h"
#include "../process/task.h"
#include "../util/memory.h"

#define __ISR_IMP(n) __attribute__((interrupt)) void isrHandler##n(InterruptFrame *, uint32_t error) { \
    term() << "EXCEPTION: " #n << " ; ERROR CODE: " << hex << error << endl; \
    asm volatile ( "cli; hlt; "); \
}

#define __ISR_IMP_NE(n) __attribute__((interrupt)) void isrHandler##n(InterruptFrame *) { \
    term() << "EXCEPTION: " #n << ";" << endl; \
    asm volatile ( "cli; hlt; "); \
}

__ISR_IMP_NE(0)
__ISR_IMP_NE(1)
__ISR_IMP_NE(2)
__ISR_IMP_NE(3)
__ISR_IMP_NE(4)
__ISR_IMP_NE(5)
__ISR_IMP_NE(6)
__ISR_IMP_NE(7)
__ISR_IMP(8)
__ISR_IMP_NE(9)
__ISR_IMP(10)
__ISR_IMP(11)
__ISR_IMP(12)

__attribute__((interrupt)) void isrHandler13(InterruptFrame *, uint32_t error) {
    term() << "Generic Protection Fault" << endl;
    if (error > 0) {
        if (error & 1) {
            term() << "External" << endl;
        }
        switch ((error >> 1) & 3) {
        case 0:
            term() << "Gdt ";
            break;
        case 1:
        case 3:
            term() << "Idt ";
            break;
        case 2:
            term() << "Ldt ";
            break;
        }
        term() << hex << (error >> 3) << endl;
    }
    asm volatile ( "cli; hlt; ");
}


__attribute__((interrupt)) void isrHandler14(InterruptFrame *, uint32_t error) {
    uint32_t cr2, cr3;
    asm volatile ("movl %%cr2, %%eax; movl %%eax, %0;":"=m"(cr2)::"%eax");
    asm volatile ("movl %%cr3, %%eax; movl %%eax, %0;":"=m"(cr3)::"%eax");
    if (((error | 2) & 7) == 6 && cr2 >= (1 << 27) && cr2 < (1 << 30)) {
        term() << "Resolve page fault!" << endl;
        currentTask->mapPage(cr2);
    } else {
        term() << "Page fault: " << error << ' ' << hex << cr2 << ' ' << cr3 << dec << endl;
        asm volatile ( "cli; hlt; ");
    }
}

__ISR_IMP_NE(15)
__ISR_IMP_NE(16)
__ISR_IMP(17)
__ISR_IMP_NE(18)
__ISR_IMP_NE(19)
__ISR_IMP_NE(20)
__ISR_IMP(21)
__ISR_IMP_NE(22)
__ISR_IMP_NE(23)
__ISR_IMP_NE(24)
__ISR_IMP_NE(25)
__ISR_IMP_NE(26)
__ISR_IMP_NE(27)
__ISR_IMP_NE(28)
__ISR_IMP(29)
__ISR_IMP(30)
__ISR_IMP_NE(31)
