#pragma once

#include <sys/types.h>
#include <bits/waitflags.h>
#include <bits/waitstatus.h>

#ifdef __cplusplus
extern "C" {
#endif

pid_t wait(int *wstatus);
pid_t waitpid(pid_t pid, int *wstatus, int options);

#ifdef __cplusplus
}
#endif
