#pragma once

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct stat {
    off_t st_size;
};

int stat(const char *pathname, struct stat *statbuf);

#ifdef __cplusplus
}
#endif
