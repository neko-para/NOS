#include "task.h"
#include "task_control_block.h"
#include "../boot/page.h"
#include "../boot/tss.h"
#include "../io/term.h"
#include "../util/memory.h"
#include "../util/new.h"

#include "../util/debug.h"

TaskControlBlock *taskCurrent;
static TaskControlBlockList *ready;
static uint32_t ntid;

template<> uint32_t PostponeScheduleLock::count = 0;
static uint32_t postpone;

void _PostponeScheduleLock::lock() {
}

void _PostponeScheduleLock::unlock() {
    if (postpone) {
        postpone = 0;
        Task::schedule();
    }
}

void Task::lock() {
    InterruptLock::lock();
}

void Task::unlock() {
    InterruptLock::unlock();
}

void Task::block() {
    lock();
    taskCurrent->setRunningState(TaskControlBlock::BLOCKING);
    schedule();
    unlock();
}

void Task::unblock(TaskControlBlock *task) {
    lock();
    task->setRunningState(TaskControlBlock::READYTORUN);
    ready->insertByPriority(task);
    schedule();
    unlock();
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

TaskControlBlock *prepare_task(uint32_t entry, uint32_t cr3, uint32_t param, uint32_t prio) {
    TaskControlBlock *task = new TaskControlBlock;

    task->esp = prepare_stack(entry, param); // 4K task stack
    task->cr3 = cr3;
    task->next = 0;
    task->state = TaskControlBlock::READYTORUN;
    task->tid = ++ntid;
    task->kesp = (task->esp & ~0xFFF) + 0x100; // 1K interrupt stack (if enter usermode)
    task->priority = prio;

    task->storePage(task->esp & ~0xFFF);
    return task;
}

static void idle() {
    Task::unlock();

    while (true) {
        hlt();
        Task::lockSchedule();
    }
}

void Task::init(void (*entry)()) {
    TaskControlBlock temp;

    taskCurrent = &temp;

    ready = new TaskControlBlockList;

    TaskControlBlock *task = prepare_task(reinterpret_cast<uint32_t>(entry), flatPage->cr3(), 0, 10);
    task->setRunningState(TaskControlBlock::RUNNING);
    sysTss.esp0 = task->kesp;
    
    TaskControlBlock *idle = prepare_task(reinterpret_cast<uint32_t>(entry), flatPage->cr3(), 0, 0);
    ready->pushBack(idle);

    switchTask(task);
}

void Task::create(void (*entry)(uint32_t), uint32_t cr3, uint32_t param, uint32_t prio) {
    if (cr3 == 0) {
        cr3 = flatPage->cr3();
    }

    TaskControlBlock *task = prepare_task(reinterpret_cast<uint32_t>(entry), cr3, param, prio);

    ready->insertByPriority(task);
    schedule();
}

void Task::exit() {
    // although we're using its page as stack, freeing it doesn't make it invalid
    if (taskCurrent->cr3 != flatPage->cr3()) {
        Page(taskCurrent->cr3).free();
    }
    delete taskCurrent;
    TaskControlBlock *task = ready->popFront();
    task->setRunningState(TaskControlBlock::RUNNING);
    switchTask(task);
}

bool Task::schedule() {
    if (PostponeScheduleLock::locked()) {
        postpone = 1;
    }
    if (ready->head->priority < taskCurrent->priority && taskCurrent->getRunningState() == TaskControlBlock::RUNNING) {
        return false;
    }
    taskCurrent->setRunningState(TaskControlBlock::READYTORUN);
    ready->insertByPriority(taskCurrent);
    TaskControlBlock *task = ready->popFront();
    task->setRunningState(TaskControlBlock::RUNNING);
    sysTss.esp0 = task->kesp;
    switchTask(task);
    return true;
}

bool Task::lockSchedule() {
    bool x;
    lock();
    x = schedule();
    unlock();
    return x;
}

static uint32_t prepare_user_stack(uint32_t entry, uint32_t virAddr) {
    uint32_t *stack = reinterpret_cast<uint32_t *>(reinterpret_cast<uint32_t>(Frame::allocUpper()) + (1 << 12));
    *--stack = 0x23; // data seg
    *--stack = virAddr + 0x1000; // esp
    *--stack = geteflags(); // eflags
    *--stack = 0x1B; // code seg
    *--stack = entry;
    return reinterpret_cast<uint32_t>(stack);
}

void Task::enterRing3(void (*entry)()) {
    Page page(reinterpret_cast<uint32_t *>(taskCurrent->cr3));
    uint32_t e = reinterpret_cast<uint32_t>(entry);
    page.autoSet(e, e, Page::PRESENT | Page::NON_SUPERVISOR | Page::READWRITE);
    uint32_t stack = prepare_user_stack(reinterpret_cast<uint32_t>(entry), (1 << 30) - 0x1000);
    taskCurrent->mapPage((1 << 30) - 0x1000, stack & ~0xFFF);
    switchRing3((1 << 30) - 4 * 5);
}

static void enterElf(uint32_t param) {
    Task::unlock();

    switchRing3((1 << 30) - 20);

/*
    Page page(reinterpret_cast<uint32_t *>(taskCurrent->cr3));
    uint32_t stack = prepare_user_stack(reinterpret_cast<uint32_t>(param));
    page.autoSet(stack, stack, Page::PRESENT | Page::NON_SUPERVISOR | Page::READWRITE);
    switchRing3(stack);
*/
}

void Task::loadELF(ELF *elf, uint32_t prio) {
    uint32_t npage = elf->countPageNeeded();
    uint32_t *phyPages = new uint32_t[npage];
    for (uint32_t i = 0; i < npage; i++) {
        phyPages[i] = reinterpret_cast<uint32_t>(Frame::allocUpper());
        taskCurrent->storePage(phyPages[i]); // TODO: use TCB::pages instead
    }
    Page page;
    elf->preparePage(&page, phyPages);
    uint32_t stack = prepare_user_stack(elf->header->entry, (1 << 30) - 0x1000);
    page.autoSet(stack, (1 << 30) - 0x1000, Page::PRESENT | Page::READWRITE | Page::NON_SUPERVISOR);
    taskCurrent->storePage(stack & ~0xFFF);
    create(enterElf, page.cr3(), stack, prio);
}