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

    void setRunningState(uint32_t s) {
        state = (state & RS_MASK) | s;
    }
};

extern TaskControlBlock *taskCurrent;

extern "C" void switchTask(TaskControlBlock *target);

class Task {
public:
    static void lock();
    static void unlock();

    static void init(void (*entry)());
    static void create(void (*entry)());
    static void exit();
    static bool schedule();
};