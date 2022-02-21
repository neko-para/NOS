#pragma once

#include <stdint.h>
#include "elf.h"

struct TaskControlBlock {
    enum {
        RUNNING = 0,
        READYTORUN = 1,
        BLOCKING = 2,
        RS_MASK = 3
    };

    uint32_t esp;
    uint32_t cr3;
    TaskControlBlock *next;
    uint32_t state;
    uint32_t tid;
    uint32_t kesp;
    uint32_t *pages;
    uint32_t npage, npageCapa;

    TaskControlBlock();
    TaskControlBlock(const TaskControlBlock &) = delete;
    ~TaskControlBlock();
    TaskControlBlock &operator=(const TaskControlBlock &) = delete;

    void setRunningState(uint32_t s) {
        state = (state & RS_MASK) | s;
    }
    void mapPage(uint32_t virAddr);
    void mapPage(uint32_t virAddr, uint32_t phyAddr);
    void storePage(uint32_t page);
};

extern TaskControlBlock *taskCurrent;

extern "C" void switchTask(TaskControlBlock *target);
extern "C" void switchRing3(uint32_t targetSp);

class Task {
public:
    static void lock();
    static void unlock();

    static void init(void (*entry)());
    static void create(void (*entry)(uint32_t), uint32_t cr3 = 0, uint32_t param = 0);
    static void exit();
    static bool schedule();
    static bool lockSchedule();

    static void enterRing3(void (*entry)());
    static void loadELF(ELF *elf);
};