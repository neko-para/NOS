#include "task.h"
#include "../boot/page.h"
#include "../boot/tss.h"
#include "../io/term.h"
#include "../io/io.h"
#include "../util/memory.h"
#include "../util/new.h"
#include "../util/string.h"

#include "../util/debug.h"

TaskControlBlock *taskCurrent;
TaskControlBlock *ready, *readyEnd;
uint32_t ntid;

TaskControlBlock::TaskControlBlock() {
    pages = 0;
    npage = 0;
    npageCapa = 0;
}

TaskControlBlock::~TaskControlBlock() {
    if (pages) {
        for (uint32_t i = 0; i < npage; i++) {
            Frame::free(reinterpret_cast<void *>(pages[i]));
        }
        delete[] pages;
    }
}

void TaskControlBlock::mapPage(uint32_t virAddr) {
    mapPage(virAddr, reinterpret_cast<uint32_t >(Frame::allocUpper()));
}

void TaskControlBlock::mapPage(uint32_t virAddr, uint32_t phyAddr) {
    Page page(cr3);
    if (!page.isSet(virAddr)) {
        storePage(phyAddr);
        page.autoSet(phyAddr, virAddr, Page::PRESENT | Page::NON_SUPERVISOR | Page::READWRITE);
    }
}

void TaskControlBlock::storePage(uint32_t page) {
    page &= ~0xFFF;
    if (npage < npageCapa) {
        pages[npage++] = page;
    } else if (npageCapa == 0) {
        pages = new uint32_t[4];
        pages[0] = page;
        npage = 1;
        npageCapa = 4;
    } else {
        npageCapa <<= 1;
        uint32_t *p = new uint32_t[npageCapa];
        memcpy(p, pages, 4 * npage);
        delete[] pages;
        pages = p;
        pages[npage++] = page;
    }
}

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

    task->storePage(task->esp & ~0xFFF);
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
    // although we're using its page as stack, freeing it doesn't make it invalid
    if (taskCurrent->cr3 != flatPage->cr3()) {
        Page(taskCurrent->cr3).free();
    }
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

void Task::loadELF(ELF *elf) {
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
    create(enterElf, page.cr3(), stack);
}