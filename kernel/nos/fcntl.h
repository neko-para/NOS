#pragma once

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

int open(const char *pathname, int flags);

#ifdef __cplusplus
}
#endif