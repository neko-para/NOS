#include <string.h>

void *memcpy(void *dst, const void *src, size_t size) {
    uint8_t *pdst = (uint8_t *)(dst), *ret = pdst;
    const uint8_t *psrc = (const uint8_t *)(src);
    while (size--) {
        *pdst++ = *psrc++;
    }
    return ret;
}

int memcmp(const void *data1, const void *data2, size_t size) {
    const uint8_t *p1 = (const uint8_t *)(data1);
    const uint8_t *p2 = (const uint8_t *)(data2);
    while (size--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return 0;
}

void *memset(void *data, uint8_t value, size_t size) {
    uint8_t *p = (uint8_t *)(data), *ret = p;
    while (size--) {
        *p++ = value;
    }
    return ret;
}

size_t strlen(const char *str) {
    size_t l = 0;
    while (*str++) {
        l++;
    }
    return l;
}

char *strcpy(char *dst, const char *src) {
    char *ret = dst;
    while (*src) {
        *dst++ = *src++;
    }
    *dst = 0;
    return ret;
}

int strcmp(const char *str1, const char *str2) {
    while (*str1 && *str2) {
        if (*str1 != *str2) {
            return *str1 - *str2;
        }
        str1++;
        str2++;
    }
    return *str1 - *str2;
}
