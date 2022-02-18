#pragma once

#include <stdint.h>

struct Tss {
    uint16_t link;
    uint16_t link_h;
    uint32_t esp0;
    uint16_t ss0;
    uint16_t ss0_h;
    uint32_t esp1;
    uint16_t ss1;
    uint16_t ss1_h;
    uint32_t esp2;
    uint16_t ss2;
    uint16_t ss2_h;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx; 
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint16_t es;
    uint16_t es_h;
    uint16_t cs;
    uint16_t cs_h;
    uint16_t ss;
    uint16_t ss_h;
    uint16_t ds;
    uint16_t ds_h;
    uint16_t fs;
    uint16_t fs_h;
    uint16_t gs;
    uint16_t gs_h;
    uint16_t ldt;
    uint16_t ldt_h;
    uint16_t trap;
    uint16_t iomap;
} __attribute__((packed));

extern Tss sysTss;