#pragma once

#include <sys/types.h>

#define DT_FIFO (1 << 12)
#define DT_CHR (2 << 12)
#define DT_DIR (4 << 12)
#define DT_BLK (6 << 12)
#define DT_REG (8 << 12)
#define DT_LNK (10 << 12)
#define DT_SOCK (12 << 12)

struct nos_dirent {
    uint16_t next;
    uint8_t length;
    uint8_t type;
    char name[0];
};

#ifdef __cplusplus
extern "C" {
#endif

int getdents(int fd, struct nos_dirent *dirp, size_t count);

#ifdef __cplusplus
}
#endif