#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct Stat {
    uint32_t size;
};

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

void exit(int32_t ret);
int32_t fork();
int32_t read(int32_t fd, void *buf, uint32_t len);
int32_t write(int32_t fd, const void *buf, uint32_t len);
int32_t open(const char *path, int32_t flag);
int32_t close(int32_t fd);
int32_t execve(const char *path, char *const argv[], char *const envp[]);
int32_t stat(const char *path, struct Stat *buf);
int32_t lseek(int32_t fd, int32_t offset, int32_t whence);
int32_t getpid();

#ifdef __cplusplus
}
#endif