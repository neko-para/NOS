#include "semaphore.h"
#include "task.h"
#include "../util/new.h"

Semaphore::Semaphore(uint32_t maxi) {
    maxCount = maxi;
    curCount = 0;
    list = new TaskControlBlockList;
}

Semaphore::~Semaphore() {
    delete list;
}

void Semaphore::lock() {
    PostponeScheduleLock::lock();
    if (curCount == maxCount) {
        list->pushBack(currentTask);
        Task::block();
    } else {
        curCount++;
    }
    PostponeScheduleLock::unlock();
}

void Semaphore::unlock() {
    PostponeScheduleLock::lock();
    if (list->isEmpty()) {
        curCount--;
    } else {
        Task::unblock(list->popFront());
    }
    PostponeScheduleLock::unlock();
}