#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

extern "C" int _main() {
    char buf[256];
    char ch = ' ';

    int fd = open("/", 0);
    while (getdents(fd, (nos_dirent *)buf, 1) > 0) {
        nos_dirent *d = (nos_dirent *)buf;
        write(1, d->name, d->length);
        write(1, &ch, 1);
    }
    ch = '\n';
    write(1, &ch, 1);
    close(fd);
    return 0;
}