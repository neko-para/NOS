#pragma once

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

void *memcpy(void *dst, const void *src, size_t size);
int memcmp(const void *data1, const void *data2, size_t size);
void *memset(void *data, uint8_t value, size_t size);

size_t strlen(const char *str);
char *strcpy(char *dst, const char *src);
int strcmp(const char *str1, const char *str2);

#ifdef __cplusplus
}
#endif
