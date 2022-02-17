#pragma once

#include <stdint.h>

class Stream {
public:
    enum {
        HEX = 1
    };

    virtual void put(char ch) {};

    Stream &operator<<(uint16_t v);
    Stream &operator<<(uint32_t v);
    Stream &operator<<(int16_t v);
    Stream &operator<<(int32_t v);
    Stream &operator<<(Stream &(*f)(Stream &));

    Stream &operator<<(void *ptr) {
        return *this << reinterpret_cast<uint32_t>(ptr);
    }

    Stream &operator<<(char c) {
        put(c);
        return *this;
    }

    Stream &operator<<(const char *s) {
        while (*s) {
            put(*s++);
        }
        return *this;
    }

    Stream &operator<<(int v) {
        return *this << int32_t(v);
    }

    void setf(uint16_t f) {
        flag = f;
    }

    uint16_t getf() const {
        return flag;
    }

private:
    uint16_t flag = 0;
};

Stream &endl(Stream &);
Stream &hex(Stream &);
Stream &dec(Stream &);