#pragma once

#include <stddef.h>

inline void *operator new(size_t, void *p) noexcept {
    return p;
}

inline void *operator new[](size_t, void *p) noexcept {
    return p;
}

inline void operator delete  (void *, void *) noexcept {
};

inline void operator delete[](void *, void *) noexcept {
};