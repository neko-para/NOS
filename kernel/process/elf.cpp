#include "elf.h"
#include "../io/term.h"
#include "../util/memory.h"
#include "../util/new.h"
#include "../util/string.h"

ELF::ELF(void *content) : content(content) {
    header = at<Header>(0);
    if (header->magic != 0x464c457F) {
        term() << "ELF MAGIC NOT MATCH!" << endl;
    }
}

ELF::~ELF() {
    Memory::free(content);
}

uint32_t ELF::countPageNeeded() {
    uint32_t req = 0;
    for (int i = 0; i < header->program_header_entry_count; i++) {
        ProgramHeader *ph = nthPH(i);
        if (ph->type == ProgramHeader::T_IGNORE) {
            continue;
        }
        if (ph->type == ProgramHeader::T_LOAD) {
            req += (ph->memsz + 0xFFF) >> 12;
        }
    }
    return req;
}

Page *ELF::preparePage(Page *page, uint32_t *phyAvl) {
    page->autoSet(0, 0, 1 << 27, Page::PRESENT | Page::READWRITE); // under 128M will be kernel's
    uint32_t x = 0;
    for (uint32_t i = 0; i < header->program_header_entry_count; i++) {
        ProgramHeader *ph = nthPH(i);
        if (ph->type == ProgramHeader::T_IGNORE) {
            continue;
        }
        if (ph->type == ProgramHeader::T_LOAD) {
            uint32_t pc = (ph->memsz + 0xFFF) >> 12;
            uint32_t pv = ph->vaddr;
            uint32_t attrib = Page::NON_SUPERVISOR | Page::PRESENT;
            if (ph->flags & ProgramHeader::F_WRITABLE) {
                attrib |= Page::READWRITE;
            }
            for (uint32_t j = 0; j < pc; j++) {
                memset(reinterpret_cast<void *>(phyAvl[x]), 0, ph->memsz);
                memcpy(reinterpret_cast<void *>(phyAvl[x]), at<void>(ph->offset), ph->filesz);
                page->autoSet(phyAvl[x++], pv, attrib, attrib);
                pv += 0x1000;
            }
        }
    }
    return page;
}

static const char *types[] = {
    "relocatable",
    "executable",
    "shared",
    "core"
};

void ELF::printInfo() {
    term() << "Magic: " << hex << header->magic << dec << endl
        << "Bit: " << (header->bit == 1 ? "32bit" : "64bit") << endl
        << "Endian: " << (header->endian == 1 ? "little" : "big") << endl
        << "Header Version: " << header->header_ver << endl
        << "OS ABI: " << header->os_abi << endl
        << "Type: " << types[header->type - 1] << endl
        << "Instruction: ";
    switch (header->machine) {
        case 0:
            term() << "No Specific";
            break;
        case 2:
            term() << "Sparc";
            break;
        case 3:
            term() << "x86";
            break;
        case 8:
            term() << "MIPS";
            break;
        case 0x14:
            term() << "PowerPC";
            break;
        case 0x28:
            term() << "ARM";
            break;
        case 0x2A:
            term() << "SuperH";
            break;
        case 0x32:
            term() << "IA-64";
            break;
        case 0x3E:
            term() << "x86-64";
            break;
        case 0xB7:
            term() << "AArch64";
            break;
        case 0xF3:
            term() << "RISC-V";
            break;
    }
    term() << endl
        << "ELF Version: " << header->version << endl
        << "Flags: " << hex << header->flags << dec << endl
        << "Program Header Entry Count: " << header->program_header_entry_count << endl
        << "Section Header Entry Count: " << header->section_header_entry_count << endl;
}
