#include "task.h"
#include "../boot/page.h"
#include "../boot/tss.h"
#include "../io/term.h"
#include "../io/io.h"
#include "../util/memory.h"
#include "../util/new.h"

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

static uint32_t prepare_stack(uint32_t entry, uint32_t param) {
    uint32_t *stack = (uint32_t *)((uint32_t)Frame::alloc() + (1 << 12));
    *--stack = param;
    *--stack = 0x00001111; // fake eip
    *--stack = entry;
    *--stack = 0; // ebx
    *--stack = 0; // esi
    *--stack = 0; // edi
    *--stack = 0; // ebp
    return reinterpret_cast<uint32_t>(stack);
}

TaskControlBlock *prepare_task(uint32_t entry, uint32_t cr3, uint32_t param) {
    TaskControlBlock *task = new TaskControlBlock;

    task->esp = prepare_stack(entry, param); // 4K task stack
    task->cr3 = cr3;
    task->next = 0;
    task->state = TaskControlBlock::READYTORUN;
    task->tid = ++ntid;
    task->kesp = (task->esp & ~0xFFF) + 0x100; // 1K interrupt stack (if enter usermode)

    return task;
}

void Task::init(void (*entry)()) {
    TaskControlBlock temp;

    taskCurrent = &temp;

    TaskControlBlock *task = prepare_task(reinterpret_cast<uint32_t>(entry), flatPage->cr3(), 0);
    task->setRunningState(TaskControlBlock::RUNNING);

    ready = readyEnd = 0;

    sysTss.esp0 = task->kesp;
    switchTask(task);
}

void Task::create(void (*entry)(uint32_t), uint32_t cr3, uint32_t param) {
    if (cr3 == 0) {
        cr3 = flatPage->cr3();
    }

    TaskControlBlock *task = prepare_task(reinterpret_cast<uint32_t>(entry), cr3, param);

    if (readyEnd) {
        readyEnd->next = task;
    } else {
        ready = task;
    }
    readyEnd = task;
}

void Task::exit() {
    while (!ready) {
        unlock();
        hlt(); // Assume that interrupt could create some task
        lock();
    }
    // although we're using this page as stack, freeing it doesn't make it invalid
    Frame::free(reinterpret_cast<void *>(reinterpret_cast<uint32_t>(taskCurrent->esp) & ~0xFFF));
    delete taskCurrent;
    TaskControlBlock *task = ready;
    ready = ready->next;
    task->setRunningState(TaskControlBlock::RUNNING);
    switchTask(task);
}

bool Task::schedule() {
    if (ready) {
        taskCurrent->setRunningState(TaskControlBlock::READYTORUN);
        readyEnd->next = taskCurrent;
        readyEnd = taskCurrent;
        taskCurrent->next = 0;
        TaskControlBlock *task = ready;
        ready = ready->next;
        task->setRunningState(TaskControlBlock::RUNNING);
        sysTss.esp0 = task->kesp;
        switchTask(task);
        return true;
    } else {
        return false;
    }
}

bool Task::lockSchedule() {
    bool x;
    lock();
    x = schedule();
    unlock();
    return x;
}

static uint32_t prepare_user_stack(uint32_t entry) {
    uint32_t bottom = reinterpret_cast<uint32_t>(Frame::alloc()) + (1 << 12);
    uint32_t *stack = reinterpret_cast<uint32_t *>(bottom);
    *--stack = 0x23; // data seg
    *--stack = bottom; // esp
    *--stack = geteflags(); // eflags
    *--stack = 0x1B; // code seg
    *--stack = entry;
    return reinterpret_cast<uint32_t>(stack);
}

void Task::enterRing3(void (*entry)()) {
    Page page(reinterpret_cast<uint32_t *>(taskCurrent->cr3));
    uint32_t e = reinterpret_cast<uint32_t>(entry);
    page.autoSet(e, e, Page::PRESENT | Page::NON_SUPERVISOR | Page::READWRITE);
    uint32_t stack = prepare_user_stack(reinterpret_cast<uint32_t>(entry));
    page.autoSet((1 << 30) - 0x1000, stack, Page::PRESENT | Page::NON_SUPERVISOR | Page::READWRITE);
    switchRing3((1 << 30) - 4 * 5);
}

static void enterElf(uint32_t param) {
    Task::unlock();

    Page page(reinterpret_cast<uint32_t *>(taskCurrent->cr3));
    uint32_t stack = prepare_user_stack(reinterpret_cast<uint32_t>(param));
    page.autoSet(stack, stack, Page::PRESENT | Page::NON_SUPERVISOR | Page::READWRITE);
    switchRing3(stack);

    Task::enterRing3(reinterpret_cast<void (*)()>(param));
}

void Task::loadELF(ELF *elf) {
    uint32_t npage = elf->countPageNeeded();
    uint32_t *phyPages = new uint32_t[npage];
    for (uint32_t i = 0; i < npage; i++) {
        phyPages[i] = reinterpret_cast<uint32_t>(Frame::alloc());
    }
    Page page;
    elf->preparePage(&page, phyPages);
    uint32_t cr3 = page.cr3();
    create(enterElf, cr3, elf->header->entry);
}