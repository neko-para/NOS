#pragma once

#include <stdint.h>

void memcpy(void *dst, void *src, uint32_t size);
int memcmp(void *data1, void *data2, uint32_t size);
void memset(void *data, uint8_t value, uint32_t size);
