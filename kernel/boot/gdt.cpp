#include "gdt.h"
#include "tss.h"

extern "C" void setGdt(void *pgdt, uint16_t size);
extern "C" void reloadSegments();

constexpr int GDTEntryCount = 6;
static uint8_t gdt_data[sizeof (Gdt) * GDTEntryCount];

void Gdt::init() {
    sysTss.ss0 = 0x10;

    Gdt *gdt = reinterpret_cast<Gdt *>(gdt_data);
    gdt[0].set(0, 0, 0, 0);
    gdt[1].set(0, 0x000FFFFF, Gdt::A_PRESENT | Gdt::A_DPL_0 | Gdt::A_NOT_SYSTEM | Gdt::A_EXECUTABLE | Gdt::A_READ_WRITE, Gdt::F_GRANULARITY | Gdt::F_SIZE);
    gdt[2].set(0, 0x000FFFFF, Gdt::A_PRESENT | Gdt::A_DPL_0 | Gdt::A_NOT_SYSTEM | Gdt::A_READ_WRITE, Gdt::F_GRANULARITY | Gdt::F_SIZE);
    gdt[3].set(0, 0x000FFFFF, Gdt::A_PRESENT | Gdt::A_DPL_3 | Gdt::A_NOT_SYSTEM | Gdt::A_EXECUTABLE | Gdt::A_READ_WRITE, Gdt::F_GRANULARITY | Gdt::F_SIZE);
    gdt[4].set(0, 0x000FFFFF, Gdt::A_PRESENT | Gdt::A_DPL_3 | Gdt::A_NOT_SYSTEM | Gdt::A_READ_WRITE, Gdt::F_GRANULARITY | Gdt::F_SIZE);
    gdt[5].set(reinterpret_cast<uint32_t>(&sysTss), sizeof (Tss), Gdt::A_PRESENT | Gdt::A_EXECUTABLE | Gdt::A_ACCESS, 0);
    setGdt(gdt, sizeof (gdt_data));
    reloadSegments();
    asm volatile ( "movw $0x28, %ax; ltr %ax;" );
}

void Gdt::setbase(uint32_t base) {
    base_lo = base & 0xFFFF;
    base_mi = (base >> 16) & 0xFF;
    base_hi = (base >> 24) & 0xFF;
}

void Gdt::setlimit(uint32_t limit) {
    limit_lo = limit & 0xFFFF;
    limit_hi = (limit >> 16) & 0xF;
}

void Gdt::set(uint32_t base, uint32_t limit, uint8_t access, uint8_t flag) {
    setbase(base);
    setlimit(limit);
    this->access = access;
    this->flag = flag;
}
