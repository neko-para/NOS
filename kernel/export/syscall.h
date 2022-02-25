#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct Stat {
    uint32_t size;
};

void exit(int32_t ret);
int32_t fork();
int32_t read(int32_t fd, void *buf, uint32_t len);
int32_t write(int32_t fd, const void *buf, uint32_t len);
int32_t open(const char *path, int32_t flag);
int32_t stat(const char *path, struct Stat *buf);

#ifdef __cplusplus
}
#endif