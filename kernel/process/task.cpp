#include "task.h"
#include "../io/io.h"
#include "../util/memory.h"

#include "../io/term.h"
#include "../util/debug.h"

TaskControlBlock *taskCurrent;
TaskControlBlock *ready, *readyEnd;
uint32_t ntid;

void Task::lock() {
    intLock();
}

void Task::unlock() {
    intUnlock();
}

static uint32_t prepare_stack(void (*entry)()) {
    uint32_t *stack = (uint32_t *)((uint32_t)Frame::alloc() + (1 << 12));
    *--stack = reinterpret_cast<uint32_t>(entry);
    *--stack = 0; // ebx
    *--stack = 0; // esi
    *--stack = 0; // edi
    *--stack = 0; // ebp
    return reinterpret_cast<uint32_t>(stack);
}

void Task::init(void (*entry)()) {
    TaskControlBlock temp;

    taskCurrent = &temp;

    TaskControlBlock *task = new TaskControlBlock;

    asm volatile ("movl %%cr3, %%eax; movl %%eax, %0;":"=m"(task->cr3)::"%eax");
    task->esp = prepare_stack(entry);
    task->next = 0;
    task->state = TaskControlBlock::RUNNING;
    task->tid = ++ntid;

    ready = readyEnd = 0;

    switchTask(task);
}

void Task::create(void (*entry)()) {
    TaskControlBlock *task = new TaskControlBlock;

    asm volatile ("movl %%cr3, %%eax; movl %%eax, %0;":"=m"(task->cr3)::"%eax");
    task->esp = prepare_stack(entry);
    task->state = TaskControlBlock::READYTORUN;
    task->next = 0;
    task->tid = ++ntid;

    if (readyEnd) {
        readyEnd->next = task;
    } else {
        ready = task;
    }
    readyEnd = task;
}

void Task::schedule() {
    if (ready) {
        taskCurrent->setRunningState(TaskControlBlock::READYTORUN);
        readyEnd->next = taskCurrent;
        readyEnd = taskCurrent;
        taskCurrent->next = 0;
        TaskControlBlock *task = ready;
        ready = ready->next;
        task->setRunningState(TaskControlBlock::RUNNING);
        switchTask(task);
    }
}