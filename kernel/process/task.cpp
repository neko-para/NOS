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
TaskControlBlock *tasks[MaxTask];
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

    task->child->pushBack(idleTask->tid);
    idleTask->parent = task->tid;

    tasks[task->tid] = task;
    tasks[idleTask->tid] = idleTask;

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
    task->parent = currentTask->tid;
    currentTask->child->pushBack(task->tid);

    tasks[task->tid] = task;

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
        if ((*currentTask->file)[i]) {
            (*task->file)[i] = (*task->file)[i]->clone();
        } else {
            (*task->file)[i] = 0;
        }
    }
    task->parent = currentTask->tid;
    currentTask->child->pushBack(task->tid);

    tasks[task->tid] = task;

    ready->insertByPriority(task);
    schedule();
    return task->tid;
}

void Task::exit(int8_t ret, int8_t sig) {
    lock();
    currentTask->param = (ret << 8) | sig;
    currentTask->setRunningState(TaskControlBlock::TERMINATED);
    auto p = tasks[currentTask->parent];
    if (p->getRunningState() == TaskControlBlock::BLOCKING && p->getBlockingReason() == TaskControlBlock::WAITCHILD) {
        if (static_cast<int32_t>(p->param) == currentTask->tid || p->param == 0) {
            p->param = currentTask->tid;
            unlock();
            unblock(p);
            return; // just incase. should never come back
        }
    }
    schedule();
}

int32_t Task::clean(int32_t tid) {
    PostponeScheduleLock::lock();
    TaskControlBlock *task = tasks[tid];
    tasks[tid] = 0;
    int32_t ret = task->param;
    if (task->cr3 != flatPage->cr3()) {
        Page(task->cr3).free();
    }
    for (uint32_t i = 0; i < task->file->size(); i++) {
        auto p = (*task->file)[i];
        if (p) {
            delete p;
        }
    }
    for (uint32_t i = 0; i < task->child->size(); i++) {
        auto id = (*task->child)[i];
        tasks[id]->parent = 1;
        tasks[1]->child->pushBack(id);
    }
    delete task;
    PostponeScheduleLock::unlock();
    return ret;
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

static uint32_t prepare_user_stack(uint32_t entry, uint32_t virAddr, uint32_t extra) {
    uint32_t bottom = reinterpret_cast<uint32_t>(Frame::allocUpper());
    uint32_t *stack = reinterpret_cast<uint32_t *>(Page::mount(bottom)) + (1 << 10);
    *--stack = extra; // argv & envp
    *--stack = 0; // fake eip
    *--stack = 0x23; // data seg
    *--stack = virAddr + 0x1000 - 8; // esp
    *--stack = geteflags(); // eflags
    *--stack = 0x1B; // code seg
    *--stack = entry;
    return reinterpret_cast<uint32_t>(bottom + (1 << 12) - 28);
}

void Task::replaceViaELF(ELF *elf, char **argv, char **envp) {
    Page prev(currentTask->cr3);
    uint32_t argpage = reinterpret_cast<uint32_t>(Frame::allocUpper());
    uint32_t mappos = (1 << 30) - 0x1000;
    if (currentTask->cr3 == flatPage->cr3()) {
        mappos = argpage;
    }
    prev.autoSet(argpage, mappos, Page::PRESENT | Page::READWRITE | Page::NON_SUPERVISOR);
    prev.load();

    char *vargsbuf = reinterpret_cast<char *>(mappos + 0x800);
    char **vargs = reinterpret_cast<char **>(mappos + 4);
    char *fake = 0;
    if (!argv) {
        argv = &fake;
    }
    if (!envp) {
        envp = &fake;
    }
    int argc = 0;
    while (*argv) {
        argc++;
        *vargs++ = vargsbuf;
        char *p = *argv++;
        while (*p) {
            *vargsbuf++ = *p++;
        }
        *vargsbuf++ = 0;
    }
    *(reinterpret_cast<int *>(mappos)) = argc;
    *vargs++ = 0;
    while (*envp) {
        *vargs++ = vargsbuf;
        char *p = *envp++;
        while (*p) {
            *vargsbuf++ = *p++;
        }
        *vargsbuf++ = 0;
    }
    *vargs++ = 0;
    if (mappos == ((1 << 30) - 0x1000)) {
        prev.autoSet(0, mappos, 0);
    }

    Page page;
    page.autoSet(0, 0, 1 << 27, Page::PRESENT | Page::READWRITE);
    page.autoSet(argpage, (1 << 30) - 0x1000, Page::PRESENT | Page::READWRITE | Page::NON_SUPERVISOR);
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
    
    currentTask->storePage(argpage);

    elf->preparePage(&page, phyPages); // will load page
    
    uint32_t stack = prepare_user_stack(elf->header->entry, (1 << 30) - 0x2000, (1 << 30) - 0x1000);

    page.autoSet(stack, (1 << 30) - 0x2000, Page::PRESENT | Page::READWRITE | Page::NON_SUPERVISOR);
    currentTask->storePage(stack & ~0xFFF);

    delete elf;

    switchRing3((1 << 30) - 0x2000 + (stack & 0xFFF));
}
