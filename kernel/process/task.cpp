#include "task.h"
#include "task_control_block.h"
#include "../boot/page.h"
#include "../boot/tss.h"
#include "../io/term.h"
#include "../io/timer.h"
#include "../util/memory.h"
#include "../util/new.h"

constexpr uint32_t DefaultTimeSlice = 50;

TaskControlBlock *currentTask;
TaskControlBlockList *sleeping;
static TaskControlBlock *idleTask;
static TaskControlBlockList *ready;
static uint32_t ntid;

extern uint32_t remainTimeSlice;

template<> uint32_t PostponeScheduleLock::count = 0;
static uint32_t postpone;

void _PostponeScheduleLock::lock() {
}

void _PostponeScheduleLock::unlock() {
    if (postpone) {
        postpone = 0;
        // Task::schedule();
        Task::lockSchedule();
    }
}

bool Task::inited = false;

void Task::lock() {
    InterruptLock::lock();
}

void Task::unlock() {
    InterruptLock::unlock();
}

void Task::block(uint32_t reason) {
    lock();
    currentTask->setBlocking(reason);
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

void Task::sleep(uint32_t ms) {
    currentTask->param = ms + Timer::msSinceBoot;
    block(TaskControlBlock::SLEEPING);
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
    task->kesp = (task->esp & ~0xFFF) + 0x800; // 2K interrupt stack (if enter usermode)
    task->priority = prio;

    task->storePage(task->esp & ~0xFFF);
    return task;
}

static void idle() {
    Task::unlock();

    while (true) {
        hlt();
    }
}

void Task::init(void (*entry)()) {
    TaskControlBlock temp;

    currentTask = &temp;

    ready = new TaskControlBlockList;
    sleeping = new TaskControlBlockList;

    TaskControlBlock *task = prepare_task(reinterpret_cast<uint32_t>(entry), flatPage->cr3(), 0, 10);
    task->setRunningState(TaskControlBlock::RUNNING);
    sysTss.esp0 = task->kesp;

    idleTask = prepare_task(reinterpret_cast<uint32_t>(idle), flatPage->cr3(), 0, 0);
    ready->pushBack(idleTask);

    sleeping = new TaskControlBlockList; // incase PIT

    switchTask(task);
}

int32_t Task::create(void (*entry)(uint32_t), uint32_t cr3, uint32_t param, uint32_t prio) {
    if (cr3 == 0) {
        cr3 = flatPage->cr3();
    }

    TaskControlBlock *task = prepare_task(reinterpret_cast<uint32_t>(entry), cr3, param, prio);

    ready->insertByPriority(task);
    schedule();
    return task->tid;
}

void backToFork(uint32_t sp) {
    Task::unlock();
    backRing3(sp, 0);
}

int32_t Task::fork(uint32_t sp) {
    Page page(currentTask->cr3);
    TaskControlBlock *task = new TaskControlBlock;
    page.checkPresent();
    uint32_t cr3 = page.clone(task);
    return create(backToFork, cr3, sp, currentTask->priority);
}

void Task::exit(int32_t ret) {
    lock();
    // although we're using its page as stack, freeing it doesn't make it invalid
    if (currentTask->cr3 != flatPage->cr3()) {
        Page(currentTask->cr3).free();
    }
    delete currentTask;
    TaskControlBlock *task = ready->popFront();
    switchWrap(task);
}

void Task::switchWrap(TaskControlBlock *task) {
    task->setRunningState(TaskControlBlock::RUNNING);
    if (task == idleTask) {
        remainTimeSlice = 0;
    } else {
        remainTimeSlice = DefaultTimeSlice;
    }
    sysTss.esp0 = task->kesp;
    switchTask(task);
}

bool Task::schedule() {
    if (PostponeScheduleLock::locked()) {
        postpone = 1;
        return false;
    }
    if (currentTask != idleTask && ready->head->priority < currentTask->priority && currentTask->getRunningState() == TaskControlBlock::RUNNING) {
        remainTimeSlice = DefaultTimeSlice;
        return false;
    }
    if (currentTask->getRunningState() == TaskControlBlock::RUNNING) {
        currentTask->setRunningState(TaskControlBlock::READYTORUN);
        ready->insertByPriority(currentTask);
    } else if (currentTask->getRunningState() == TaskControlBlock::BLOCKING && currentTask->getBlockingReason() == TaskControlBlock::SLEEPING) {
        sleeping->pushBack(currentTask);
    }
    TaskControlBlock *task = ready->popFront();
    switchWrap(task);
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
    Page page(reinterpret_cast<uint32_t *>(currentTask->cr3));
    uint32_t e = reinterpret_cast<uint32_t>(entry);
    page.autoSet(e, e, Page::PRESENT | Page::NON_SUPERVISOR | Page::READWRITE);
    uint32_t stack = prepare_user_stack(reinterpret_cast<uint32_t>(entry), (1 << 30) - 0x1000);
    currentTask->mapPage((1 << 30) - 0x1000, stack & ~0xFFF);
    switchRing3((1 << 30) - 4 * 5);
}

static void enterElf(uint32_t param) {
    Task::unlock();

    switchRing3((1 << 30) - 20);

/*
    Page page(reinterpret_cast<uint32_t *>(currentTask->cr3));
    uint32_t stack = prepare_user_stack(reinterpret_cast<uint32_t>(param));
    page.autoSet(stack, stack, Page::PRESENT | Page::NON_SUPERVISOR | Page::READWRITE);
    switchRing3(stack);
*/
}

int32_t Task::loadELF(ELF *elf, uint32_t prio) {
    uint32_t npage = elf->countPageNeeded();
    uint32_t *phyPages = new uint32_t[npage];
    for (uint32_t i = 0; i < npage; i++) {
        phyPages[i] = reinterpret_cast<uint32_t>(Frame::allocUpper());
        currentTask->storePage(phyPages[i]); // TODO: use TCB::pages instead
    }
    Page page;
    elf->preparePage(&page, phyPages);
    uint32_t stack = prepare_user_stack(elf->header->entry, (1 << 30) - 0x1000);
    page.autoSet(stack, (1 << 30) - 0x1000, Page::PRESENT | Page::READWRITE | Page::NON_SUPERVISOR);
    currentTask->storePage(stack & ~0xFFF);
    return create(enterElf, page.cr3(), stack, prio);
}