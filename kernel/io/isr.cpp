#include "isr.h"
#include "../io/term.h"
#include "../process/task.h"
#include "../util/memory.h"

#define __ISR_IMP(n) __attribute__((interrupt)) void isrHandler##n(InterruptFrame *frame, uint32_t error) { \
    term() << "EXCEPTION: " #n << " ; ERROR CODE: " << hex << error << endl; \
    asm volatile ( "cli; hlt; "); \
}

__ISR_IMP(0)
__ISR_IMP(1)
__ISR_IMP(2)
__ISR_IMP(3)
__ISR_IMP(4)
__ISR_IMP(5)
__ISR_IMP(6)
__ISR_IMP(7)
__ISR_IMP(8)
__ISR_IMP(9)
__ISR_IMP(10)
__ISR_IMP(11)
__ISR_IMP(12)

__attribute__((interrupt)) void isrHandler13(InterruptFrame *frame, uint32_t error) {
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


__attribute__((interrupt)) void isrHandler14(InterruptFrame *frame, uint32_t error) {
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

__ISR_IMP(15)
__ISR_IMP(16)
__ISR_IMP(17)
__ISR_IMP(18)
__ISR_IMP(19)
__ISR_IMP(20)
__ISR_IMP(21)
__ISR_IMP(22)
__ISR_IMP(23)
__ISR_IMP(24)
__ISR_IMP(25)
__ISR_IMP(26)
__ISR_IMP(27)
__ISR_IMP(28)
__ISR_IMP(29)
__ISR_IMP(30)
__ISR_IMP(31)
