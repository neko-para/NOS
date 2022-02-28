#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>

long syscall(long number, ...) {
    long ebx, ecx, edx, ret;
    va_list ap;

    va_start(ap, number);
    ebx = va_arg(ap, long);
    ecx = va_arg(ap, long);
    edx = va_arg(ap, long);
    va_end(ap);
    asm volatile ( "movl %0, %%ebx; movl %1, %%ecx; movl %2, %%edx; movl %3, %%eax; int $0x80; movl %%eax, %4; " : "=m"(ebx), "=m"(ecx), "=m"(edx), "=m"(number), "=m"(ret) );
    return ret;
}

void _exit(int status) {
    syscall(SYS_exit, status);
    // shall not back here
    while (1) {
        ;
    }
}

pid_t fork(void) {
    return syscall(SYS_fork);
}

ssize_t read(int fd, void *buf, size_t count) {
    return syscall(SYS_fork, fd, buf, count);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return syscall(SYS_write, fd, buf, count);
}

int close(int fd) {
    return syscall(SYS_close, fd);
}

int execve(const char *pathname, char *const argv[], char *const envp[]) {
    return syscall(SYS_execve, pathname, argv, envp);
}

off_t lseek(int fildes, off_t offset, int whence) {
    return syscall(SYS_lseek, fildes, offset, whence);
}

pid_t getpid(void) {
    return syscall(SYS_getpid);
}

/* sys/stat.h */

int stat(const char *pathname, struct stat *statbuf) {
    return syscall(SYS_stat, pathname, statbuf);
}

/* fcntl.h */

int open(const char *pathname, int flags) {
    return syscall(SYS_open, pathname, flags);
}

/* sys/wait.h */

pid_t wait(int *wstatus) {
    return waitpid(-1, wstatus, 0);
}

pid_t waitpid(pid_t pid, int *wstatus, int options) {
    return syscall(SYS_waitpid, pid, wstatus, options);
}
