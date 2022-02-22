#include "idt.h"
#include "io.h"
#include "term.h"
#include "timer.h"
#include "../process/task.h"

constexpr uint32_t timeBetweenTicks = 1;

uint64_t Timer::msSinceBoot = 0;

void Timer::set(uint32_t hz) {
    cli();
    uint16_t h = DEFAULT_HZ / hz;
    outb(0x43, 0x34);
    outb(0x40, h & 0xFF);
    outb(0x40, h >> 8);
}

extern TaskControlBlockList *sleeping;
uint32_t remainTimeSlice;

bool checkSleeing(TaskControlBlock *task) {
    if (task->param <= Timer::msSinceBoot) {
        task->param = 0;
        Task::unblock(task);
        return true;
    } else {
        return false;
    }
}

__attribute__((interrupt)) void isrHandler32(InterruptFrame *frame) {
    Idt::end(0);
    Timer::msSinceBoot += timeBetweenTicks;
    if (Task::inited) {
        PostponeScheduleLock::lock();
        sleeping->filter(checkSleeing);
        if (remainTimeSlice != 0) {
            if (remainTimeSlice <= timeBetweenTicks) {
                Task::schedule();
            } else {
                remainTimeSlice -= timeBetweenTicks;
            }
        }
        PostponeScheduleLock::unlock();
    }
}