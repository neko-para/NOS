#include <unistd.h>

extern int _main(int argc, char *argv[], char *envp[]);

void _start(char **env) {
    int argc;
    char **argv;
    
    argc = *((int *)env);
    env++;
    argv = env;
    env += argc;
    if (*env != 0) {
        _exit(-1);
    }
    _exit(_main(argc, argv, ++env));
}