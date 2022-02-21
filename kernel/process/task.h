#pragma once

#include <stdint.h>
#include "elf.h"
#include "../io/io.h"
#include "task_control_block.h"

struct _PostponeScheduleLock {
    static void lock();
    static void unlock();
};

using PostponeScheduleLock = CountingLock<_PostponeScheduleLock>;

class Task {
public:
    static void lock();
    static void unlock();

    static void block();
    static void unblock(TaskControlBlock *task);

    static void init(void (*entry)());
    static void create(void (*entry)(uint32_t), uint32_t cr3 = 0, uint32_t param = 0, uint32_t prio = 10);
    static void exit();
    static void switchWrap(TaskControlBlock *task);
    static bool schedule();
    static bool lockSchedule();

    static void enterRing3(void (*entry)());
    static void loadELF(ELF *elf, uint32_t prio = 10);
};