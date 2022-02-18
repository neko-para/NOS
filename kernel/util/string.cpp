#include "string.h"

void memcpy(void *dst, void *src, uint32_t size) {
    uint8_t *pdst = reinterpret_cast<uint8_t *>(dst);
    uint8_t *psrc = reinterpret_cast<uint8_t *>(src);
    while (size--) {
        *pdst++ = *psrc++;
    }
}

int memcmp(const void *data1, const void *data2, uint32_t size) {
    const uint8_t *p1 = reinterpret_cast<const uint8_t *>(data1);
    const uint8_t *p2 = reinterpret_cast<const uint8_t *>(data2);
    while (size--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return 0;
}

void memset(void *data, uint8_t value, uint32_t size) {
    uint8_t *p = reinterpret_cast<uint8_t *>(data);
    while (size--) {
        *p++ = value;
    }
}

uint32_t strlen(const char *str) {
    uint32_t l = 0;
    while (*str++) {
        l++;
    }
    return l;
}