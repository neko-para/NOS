#include "string.h"

void memcpy(void *dst, void *src, uint32_t size) {
    uint8_t *pdst = reinterpret_cast<uint8_t *>(dst);
    uint8_t *psrc = reinterpret_cast<uint8_t *>(src);
    while (size--) {
        *pdst++ = *psrc++;
    }
}