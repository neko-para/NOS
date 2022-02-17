#pragma once

#include <stdint.h>

class Memory {
public:
    static void init();

    static void add(uint32_t start, uint32_t size);

    static void *alloc(uint32_t size);
    static void free(void *ptr);
};