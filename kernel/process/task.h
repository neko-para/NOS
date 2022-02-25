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
    static void exit(int32_t ret = 0);
    static void switchWrap(TaskControlBlock *task);
    static bool schedule();
    static bool lockSchedule();

    static void enterRing3(void (*entry)());
    static int32_t loadELF(ELF *elf, uint32_t prio = 10);

    static bool inited;
};

extern TaskControlBlock *tasks[65536];
