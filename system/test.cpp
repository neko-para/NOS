#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

extern "C" int _main() {
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
        char path[] = "/bin/echo";
        char text[] = "Hello world!";
        char *args[] = {
            path,
            text,
            0
        };
        execve(path, args, 0);
    }
    return 0;
}