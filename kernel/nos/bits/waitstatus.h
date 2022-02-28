#pragma once

#define WTERMSIG(wstatus) ((wstatus) & 0x7F)
#define WEXITSTATUS(wstatus) (((wstatus) >> 8) & 0xFF)

#define WIFEXITED(wstatus) (WTERMSIG(wstatus) == 0)
#define WIFSIGNALED(wstatus) (((signed char) (((wstatus) & 0x7f) + 1) >> 1) > 0)