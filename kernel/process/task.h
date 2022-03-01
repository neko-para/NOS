#pragma once

#include <stdint.h>
#include "elf.h"
#include "task_control_block.h"
#include "../io/io.h"

struct _PostponeScheduleLock {
    static void lock();
    static void unlock();
};

using PostponeScheduleLock = CountingLock<_PostponeScheduleLock>;

class Task {
public:
    static void lock();
    static void unlock();

    static void block(uint32_t reason = TaskControlBlock::WAITING);
    static void unblock(TaskControlBlock *task);
    static void sleep(uint32_t ms);

    static void init(void (*entry)());
    static int32_t create(void (*entry)(uint32_t), uint32_t cr3 = 0, uint32_t param = 0, uint32_t prio = 10);
    static int32_t fork(uint32_t entry);
    static void exit(int8_t ret = 0, int8_t sig = 0);
    static int32_t clean(int32_t tid);
    static void switchWrap(TaskControlBlock *task);
    static bool schedule();
    static bool lockSchedule();

    static void replaceViaELF(ELF *elf, char **argv = 0, char **envp = 0);

    static bool inited;
};

constexpr int32_t MaxTask = 65536;

extern TaskControlBlock *tasks[MaxTask];
