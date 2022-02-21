#pragma once

#include <stdint.h>

inline void outb(uint16_t port, uint8_t val) {
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

inline void outw(uint16_t port, uint16_t val) {
    asm volatile ( "outw %0, %1" : : "a"(val), "Nd"(port) );
}

inline void outl(uint16_t port, uint32_t val) {
    asm volatile ( "outl %0, %1" : : "a"(val), "Nd"(port) );
}

inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile ( "inw %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    asm volatile ( "inl %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

inline void cli() {
    asm volatile ( "cli" );
}

inline void sti() {
    asm volatile ( "sti" );
}

inline void hlt() {
    asm volatile ( "hlt" );
}

inline uint32_t geteflags() {
    uint32_t f;
    asm volatile ( "pushf; popl %0" : "=a"(f) );
    return f;
}

inline void seteflags(uint32_t f) {
    asm volatile ( "pushl %0; popf;" : "=m"(f) );
}

inline bool isIntEnabled() {
    return geteflags() & (1 << 9);
}

template <typename Type>
struct CountingLock {
public:
    static void lock() {
        if (++count == 1) {
            Type::lock();
        }
    }
    static void unlock() {
        if (--count == 0) {
            Type::unlock();
        }
    }
    static bool locked() {
        return count > 0;
    }

private:
    static uint32_t count;
};

struct _InterruptLock {
    static void lock();
    static void unlock();
};

using InterruptLock = CountingLock<_InterruptLock>;