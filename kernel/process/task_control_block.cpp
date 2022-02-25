#include "task.h"
#include "task_control_block.h"
#include "../boot/page.h"
#include "../util/memory.h"
#include "../util/new.h"
#include "../util/string.h"

TaskControlBlock::TaskControlBlock() {
    pages = 0;
    npage = 0;
    npageCapa = 0;
    file = new Array<VFS::FileDescriptor *>();
}

TaskControlBlock::~TaskControlBlock() {
    if (pages) {
        for (uint32_t i = 0; i < npage; i++) {
            Frame::free(reinterpret_cast<void *>(pages[i]));
        }
        delete[] pages;
    }
    for (uint32_t i = 0; i < file->size(); i++) {
        delete (*file)[i];
    }
    delete file;
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

void TaskControlBlockList::pushBack(TaskControlBlock *task) {
    task->next = 0;
    if (head) {
        tail->next = task;
        tail = task;
    } else {
        head = tail = task;
    }
}

TaskControlBlock *TaskControlBlockList::popFront() {
    if (head) {
        TaskControlBlock *task = head;
        if (task == tail) {
            head = tail = 0;
        } else {
            head = task->next;
        }
        return task;
    } else {
        return 0;
    }
}

void TaskControlBlockList::insertByPriority(TaskControlBlock *task) {
    TaskControlBlock **ptr = &head;
    while (*ptr) {
        if (task->priority > (*ptr)->priority) {
            task->next = *ptr;
            *ptr = task;
            return;
        }
        ptr = &((*ptr)->next);
    }
    task->next = 0;
    tail = task;
    *ptr = task;
}

void TaskControlBlockList::filter(bool (*func)(TaskControlBlock *task)) {
    if (isEmpty()) {
        return;
    }
    if (head == tail) {
        if (func(head)) {
            head = tail = 0;
        }
        return;
    }
    TaskControlBlock *newhead = 0, *newtail = 0;
    TaskControlBlock *ptr = head;
    while (ptr) {
        TaskControlBlock *next = ptr->next;
        if (func(ptr)) {
            ptr = next;
        } else {
            if (newhead) {
                newtail->next = ptr;
            } else {
                newhead = ptr;
            }
            newtail = ptr;
            ptr = next;
        }
    }
    head = newhead;
    tail = newtail;
    if (tail) {
        tail->next = 0;
    }
}
