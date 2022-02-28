#include <sys/wait.h>
#include <unistd.h>

void strcpy(char *dst, const char *src) {
    while (*src) {
        *dst++ = *src++;
    }
    *dst = 0;
}

uint32_t strlen(const char *src) {
    uint32_t l = 0;
    while (*src++) {
        l++;
    }
    return l;
}

extern "C" void _start() {
    write(1, "From parent!\n", 13);
    pid_t p = fork();
    if (p) { // parent
        int ws;
        char buf[] = "child exited 0!\n";
        waitpid(p, &ws, 0);
        buf[13] = WEXITSTATUS(ws) + '0';
        write(1, buf, 16);
        write(1, "parent exit!\n", 13);
    } else {
        execve("/bin/about", 0, 0);
    }
    _exit(0);
}