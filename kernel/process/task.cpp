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
TaskControlBlock *tasks[65536];
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
    tasks[task->tid] = task;
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
    task->file = new Array<VFS::FileDescriptor *>();
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
    task->file = new Array<VFS::FileDescriptor *>(*currentTask->file);
    for (uint32_t i = 0; i < task->file->size(); i++) {
        (*task->file)[i] = (*task->file)[i]->clone();
    }

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
    TaskControlBlock *task = prepare_task(reinterpret_cast<uint32_t>(backToFork), 0, sp, currentTask->priority);
    uint32_t cr3 = page.clone(task);
    task->cr3 = cr3;
    task->file = new Array<VFS::FileDescriptor *>(*currentTask->file);
    for (uint32_t i = 0; i < task->file->size(); i++) {
        (*task->file)[i] = (*task->file)[i]->clone();
    }

    ready->insertByPriority(task);
    schedule();
    return task->tid;
}

void Task::exit(int32_t ret) {
    lock();
    // although we're using its page as stack, freeing it doesn't make it invalid
    if (currentTask->cr3 != flatPage->cr3()) {
        Page(currentTask->cr3).free();
    }
    tasks[currentTask->tid] = 0;
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
    uint32_t bottom = reinterpret_cast<uint32_t>(Frame::allocUpper());
    uint32_t *stack = reinterpret_cast<uint32_t *>(Page::mount(bottom)) + (1 << 10);
    *--stack = 0x23; // data seg
    *--stack = virAddr + 0x1000; // esp
    *--stack = geteflags(); // eflags
    *--stack = 0x1B; // code seg
    *--stack = entry;
    return reinterpret_cast<uint32_t>(bottom + (1 << 12) - 20);
}

static void enterElf(uint32_t param) {
    Task::unlock();

    switchRing3((1 << 30) - 20);
}

int32_t Task::createViaELF(ELF *elf, uint32_t prio) {
    Page page;
    uint32_t stack = prepare_user_stack(elf->header->entry, (1 << 30) - 0x1000);
    TaskControlBlock *task = prepare_task(reinterpret_cast<uint32_t>(enterElf), page.cr3(), stack, prio);

    uint32_t npage = elf->countPageNeeded();
    uint32_t *phyPages = new uint32_t[npage];
    for (uint32_t i = 0; i < npage; i++) {
        phyPages[i] = reinterpret_cast<uint32_t>(Frame::allocUpper());
        task->storePage(phyPages[i]); // TODO: use TCB::pages instead
    }
    page.autoSet(0, 0, 1 << 27, Page::PRESENT | Page::READWRITE); // under 128M will be kernel's
    elf->preparePage(&page, phyPages);
    page.autoSet(stack, (1 << 30) - 0x1000, Page::PRESENT | Page::READWRITE | Page::NON_SUPERVISOR);
    task->storePage(stack & ~0xFFF);
    task->file = new Array<VFS::FileDescriptor *>(*currentTask->file);
    for (uint32_t i = 0; i < task->file->size(); i++) {
        (*task->file)[i] = (*task->file)[i]->clone();
    }

    ready->insertByPriority(task);
    schedule();
    return task->tid;
}

void Task::replaceViaELF(ELF *elf) {
    Page page;
    uint32_t stack = prepare_user_stack(elf->header->entry, (1 << 30) - 0x1000);

    page.autoSet(0, 0, 1 << 27, Page::PRESENT | Page::READWRITE);
    page.load();

    if (currentTask->cr3 != flatPage->cr3()) {
        Page(currentTask->cr3).free();
        for (uint32_t i = 0; i < currentTask->npage; i++) {
            Frame::free(reinterpret_cast<void *>(currentTask->pages[i]));
        }
        delete[] currentTask->pages;
        currentTask->pages = 0;
        currentTask->npage = 0;
    }

    uint32_t npage = elf->countPageNeeded();
    uint32_t *phyPages = new uint32_t[npage];
    for (uint32_t i = 0; i < npage; i++) {
        phyPages[i] = reinterpret_cast<uint32_t>(Frame::allocUpper());
        currentTask->storePage(phyPages[i]);
    }
    currentTask->cr3 = page.cr3();
    elf->preparePage(&page, phyPages); // will load page
    page.autoSet(stack, (1 << 30) - 0x1000, Page::PRESENT | Page::READWRITE | Page::NON_SUPERVISOR);
    currentTask->storePage(stack & ~0xFFF);

    delete elf;

    switchRing3((1 << 30) - 4 * 5);
}
