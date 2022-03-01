#include <unistd.h>
#include <string.h>

extern "C" int _main(int argc, char *argv[]) {
    char ch = ' ';
    for (int i = 1; i < argc; i++) {
        char *p = argv[i];
        uint32_t l = strlen(p);
        write(1, p, l);
        if (i + 1 < argc) {
            write(1, &ch, 1);
        }
    }
    ch = '\n';
    write(1, &ch, 1);
    return 0;
}