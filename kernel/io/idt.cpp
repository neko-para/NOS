#include "idt.h"
#include "io.h"
#include "isr.h"

#include "keyboard.h"
#include "mouse.h"

#define PIC1          0x20
#define PIC2          0xA0
#define PIC1_COMMAND  PIC1
#define PIC1_DATA     (PIC1+1)
#define PIC2_COMMAND  PIC2
#define PIC2_DATA     (PIC2+1)

#define PIC1_ICW1   0x0020
#define PIC1_OCW2   0x0020
#define PIC1_IMR    0x0021
#define PIC1_ICW2   0x0021
#define PIC1_ICW3   0x0021
#define PIC1_ICW4   0x0021
#define PIC2_ICW1   0x00a0
#define PIC2_OCW2   0x00a0
#define PIC2_IMR    0x00a1
#define PIC2_ICW2   0x00a1
#define PIC2_ICW3   0x00a1
#define PIC2_ICW4   0x00a1

#define ICW1_ICW4       0x01
#define ICW1_SINGLE     0x02
#define ICW1_INTERVAL4  0x04
#define ICW1_LEVEL      0x08
#define ICW1_INIT       0x10
 
#define ICW4_8086        0x01
#define ICW4_AUTO        0x02
#define ICW4_BUF_SLAVE   0x08
#define ICW4_BUF_MASTER  0x0C
#define ICW4_SFNM        0x10

#define PIC_READ_IRR  0x0a
#define PIC_READ_ISR  0x0b

__attribute__((aligned(0x10))) static IdtEntry idt[256];
static IDTR idtr;

void IdtEntry::set(uint32_t isr, uint8_t attr) {
    isr_low = isr & 0xFFFF;
    kernel_cs = 0x08;
    _ = 0;
    attribute = attr;
    isr_high = (isr >> 16) & 0xFFFF;
}

static void initPIC() {
    outb(PIC1_IMR, 0xff); // disable interrupts
    outb(PIC2_IMR, 0xff);

    outb(PIC1_ICW1, 0x11); // edge
    outb(PIC1_ICW2, 0x20); // remap 0 ~ 7 to 0x20 ~ 0x27
    outb(PIC1_ICW3, 1 << 2); // IRQ2
    outb(PIC1_ICW4, 0x01); // no buffer


    outb(PIC2_ICW1, 0x11);
    outb(PIC2_ICW2, 0x28); // remap 8 ~ 15 to 0x28 ~ 0x2f
    outb(PIC2_ICW3, 2);
    outb(PIC2_ICW4, 0x01);

    outb(PIC1_IMR, 0xff);
    outb(PIC2_IMR, 0xff);
}

void Idt::init() {
    initPIC();
    idtr.base = reinterpret_cast<uint32_t>(idt);
    idtr.limit = sizeof (idt) - 1;

#define __INT_SET(n, f) idt[n].set(reinterpret_cast<uint32_t>(f), IdtEntry::A_PRESENT | IdtEntry::A_GATE_INT_32)

#define __ISR_SET(n) __INT_SET(n, isrHandler##n)

    __ISR_SET(0);
    __ISR_SET(1);
    __ISR_SET(2);
    __ISR_SET(3);
    __ISR_SET(4);
    __ISR_SET(5);
    __ISR_SET(6);
    __ISR_SET(7);
    __ISR_SET(8);
    __ISR_SET(9);
    __ISR_SET(10);
    __ISR_SET(11);
    __ISR_SET(12);
    __ISR_SET(13);
    __ISR_SET(14);
    __ISR_SET(15);
    __ISR_SET(16);
    __ISR_SET(17);
    __ISR_SET(18);
    __ISR_SET(19);
    __ISR_SET(20);
    __ISR_SET(21);
    __ISR_SET(22);
    __ISR_SET(23);
    __ISR_SET(24);
    __ISR_SET(25);
    __ISR_SET(26);
    __ISR_SET(27);
    __ISR_SET(28);
    __ISR_SET(29);
    __ISR_SET(30);
    __ISR_SET(31);

    __ISR_SET(33);
    __ISR_SET(44);

    asm volatile ( "lidt %0":: "m"(idtr) );
}

void Idt::mask(uint8_t irq) {
    uint16_t port;
    if(irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    outb(port, inb(port) | (1 << irq));
}

void Idt::unmask(uint8_t irq) {
    uint16_t port;
    if(irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    outb(port, inb(port) & ~(1 << irq));
}

void Idt::end(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC2_COMMAND, 0x20);
    }
    outb(PIC1_COMMAND, 0x20);
}