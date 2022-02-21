#pragma once

#include <stdint.h>
#include "task_control_block.h"

class Semaphore {
public:
    Semaphore(uint32_t maxi);
    ~Semaphore();

    void lock();
    void unlock();
    uint32_t count() const {
        return curCount;
    }
private:
    uint32_t maxCount, curCount;
    TaskControlBlockList *list;
};