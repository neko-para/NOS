#pragma once

#include <stdint.h>
#include "../vfs/vfs.h"
#include "../util/array"

struct TaskControlBlock {
    enum {
        RUNNING = 0,
        READYTORUN = 1,
        BLOCKING = 2,
        TERMINATED = 3,
        RS_MASK = 3,

        WAITING = 1 << 4,
        SLEEPING = 2 << 4,
        WAITCHILD = 3 << 4,
        BR_MASK = 3 << 4
    };

    uint32_t esp;
    uint32_t cr3;
    TaskControlBlock *next;
    uint32_t state;
    int32_t tid;
    uint32_t kesp;
    uint32_t priority;
    uint32_t param;
    uint32_t *pages;
    uint32_t npage, npageCapa;
    Array<VFS::FileDescriptor *> *file;
    int32_t parent;
    Array<int32_t> *child;

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

    void setBlocking(uint32_t reason) {
        state = (state & ~(RS_MASK | BR_MASK)) | BLOCKING | reason;
    }

    uint32_t getBlockingReason() const {
        return state & BR_MASK;
    }

    void mapPage(uint32_t virAddr);
    void mapPage(uint32_t virAddr, uint32_t phyAddr);
    void storePage(uint32_t page);
};

extern TaskControlBlock *currentTask;

extern "C" void switchTask(TaskControlBlock *target);
extern "C" void switchRing3(uint32_t targetSp);
extern "C" void backRing3(uint32_t targetSp, uint32_t eax);

struct TaskControlBlockList {
    TaskControlBlock *head = 0, *tail = 0;

    bool isEmpty() {
        return !head;
    }

    void pushBack(TaskControlBlock *task);
    TaskControlBlock *popFront();
    void insertByPriority(TaskControlBlock *task); // upper to lower
    void filter(bool (*func)(TaskControlBlock *task)); // remove all true
};