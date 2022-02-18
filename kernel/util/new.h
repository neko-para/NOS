#pragma once

#ifdef VSCODE_CPPTOOL
#define __size_t
typedef unsigned int size_t;
#endif

#include <stddef.h>
#include "memory.h"

inline void *operator new(size_t, void *p) noexcept {
    return p;
}

inline void *operator new[](size_t, void *p) noexcept {
    return p;
}

inline void operator delete(void *, void *) noexcept {
};

inline void operator delete[](void *, void *) noexcept {
};

inline void *operator new(size_t s) noexcept {
    return Memory::alloc(s);
}

inline void *operator new[](size_t s) noexcept {
    return Memory::alloc(s);
}

inline void operator delete(void *ptr, size_t) noexcept {
    Memory::free(ptr);
}

inline void operator delete[](void *ptr, size_t) noexcept {
    Memory::free(ptr);
}

inline void operator delete(void *ptr) noexcept {
    Memory::free(ptr);
}

inline void operator delete[](void *ptr) noexcept {
    Memory::free(ptr);
}