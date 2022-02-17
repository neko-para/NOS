#include "isr.h"
#include "../io/term.h"

#define __ISR_IMP(n) __attribute__((interrupt)) void isrHandler##n(InterruptFrame *frame, uint32_t error) { \
    term() << "EXCEPTION: " #n << " ; ERROR CODE: " << error << endl; \
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
__ISR_IMP(13)
__ISR_IMP(14)
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
