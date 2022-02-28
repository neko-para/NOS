#pragma once

#include <sys/types.h>

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#ifdef __cplusplus
extern "C" {
#endif

long syscall(long number, ...);

void _exit(int status);
pid_t fork(void);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
int close(int fd);
int execve(const char *pathname, char *const argv[], char *const envp[]);
off_t lseek(int fildes, off_t offset, int whence);
pid_t getpid(void);

#ifdef __cplusplus
}
#endif