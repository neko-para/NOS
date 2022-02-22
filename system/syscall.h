#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void exit(int32_t ret);
int32_t fork();
void write(int32_t fd, void *buf, uint32_t len);

#ifdef __cplusplus
}
#endif