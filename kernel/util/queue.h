#pragma once

#include <stdint.h>
#include "new.h"

template <typename Type, uint32_t Size = 128>
class Queue { // Fixed size
public:
    bool empty() {
        return head == tail;
    }

    bool full() {
        return tail + 1 == head || ((head == 0) && (tail == Size - 1));
    }

    bool push(const Type &v) {
        if (full()) {
            return false;
        }
        new (&data[tail])Type(v);
        if (++tail == Size) {
            tail = 0;
        }
        return true;
    }

    bool pop(Type &v) {
        if (empty()) {
            return false;
        }
        v = data[head];
        if (++head == Size) {
            head = 0;
        }
        return true;
    }

private:
    Type data[Size];
    uint32_t head = 0, tail = 0;
};