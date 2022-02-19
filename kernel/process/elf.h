#pragma once

#include <stdint.h>
#include "../boot/page.h"

struct ELF {
    struct Header {
        uint32_t magic; // 0x464c457F
        uint8_t bit;
        uint8_t endian;
        uint8_t header_ver;
        uint8_t os_abi;
        uint8_t padding[8];
        uint16_t type;
        uint16_t machine;
        uint32_t version;
        uint32_t entry;
        uint32_t program_header_table;
        uint32_t section_header_table;
        uint32_t flags;
        uint16_t header_size;
        uint16_t program_header_entry_size;
        uint16_t program_header_entry_count;
        uint16_t section_header_entry_size;
        uint16_t section_header_entry_count;
        uint16_t name_index;
    };

    struct ProgramHeader {
        enum {
            T_IGNORE = 0,
            T_LOAD = 1,
            T_DYNAMIC = 2,
            T_INTERP = 3,
            T_NOTE = 4,

            F_EXECUTABLE = 1,
            F_WRITABLE = 2,
            F_READABLE = 4
        };
        uint32_t type;
        uint32_t offset;
        uint32_t vaddr;
        uint32_t pad;
        uint32_t filesz;
        uint32_t memsz;
        uint32_t flags;
        uint32_t align;
    };

    struct SectionHeader {
        uint32_t name;
        uint32_t type;
        uint32_t flags;
        uint32_t addr;
        uint32_t offset;
        uint32_t size;
        uint32_t link;
        uint32_t info;
        uint32_t align;
        uint32_t entry_size;
    };

    ELF(void *content); // take ownership
    ~ELF();

    ELF(const ELF &) = delete;
    ELF &operator=(const ELF &) = delete;

    uint32_t countPageNeeded();
    Page *preparePage(Page *page, uint32_t *phyAvl);

    void printInfo();

    template <typename T>
    T *at(uint32_t offset) {
        return reinterpret_cast<T *>(reinterpret_cast<uint8_t *>(content) + offset);
    }

    ProgramHeader *nthPH(uint32_t idx) {
        return at<ProgramHeader>(header->program_header_table + header->program_header_entry_size * idx);
    }

    SectionHeader *nthSH(uint32_t idx) {
        return at<SectionHeader>(header->section_header_table + header->section_header_entry_size * idx);
    }

    SectionHeader *sectionNameSection() {
        return nthSH(header->name_index);
    }

    const char *sectionName(uint32_t offset) {
        return at<char>(sectionNameSection()->offset + offset);
    }

    void *content;
    Header *header;
};