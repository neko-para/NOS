#pragma once

#include <stdint.h>

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
    uint32_t priority;
    uint32_t *pages;
    uint32_t npage, npageCapa;

    TaskControlBlock();
    TaskControlBlock(const TaskControlBlock &) = delete;
    ~TaskControlBlock();
    TaskControlBlock &operator=(const TaskControlBlock &) = delete;

    void setRunningState(uint32_t s) {
        state = (state & ~RS_MASK) | s;
    }

    uint32_t getRunningState() const {
        return state & RS_MASK;
    }

    void mapPage(uint32_t virAddr);
    void mapPage(uint32_t virAddr, uint32_t phyAddr);
    void storePage(uint32_t page);
};

extern TaskControlBlock *currentTask;

extern "C" void switchTask(TaskControlBlock *target);
extern "C" void switchRing3(uint32_t targetSp);

struct TaskControlBlockList {
    TaskControlBlock *head = 0, *tail = 0;

    bool isEmpty() {
        return !head;
    }

    void pushBack(TaskControlBlock *task);
    TaskControlBlock *popFront();
    void insertByPriority(TaskControlBlock *task); // upper to lower
};