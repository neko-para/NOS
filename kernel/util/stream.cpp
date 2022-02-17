#include "stream.h"

inline char toHex(uint8_t n) {
    return n < 10 ? n + '0' : n + 'A' - 10;
}

Stream &Stream::operator<<(uint16_t v) {
    if (flag & HEX) {
        put(toHex(v >> 12));
        put(toHex((v >> 8) & 0xF));
        put(toHex((v >> 4) & 0xF));
        put(toHex(v & 0xF));
    } else {
        char buf[7];
        int ptr = 0;
        do {
            buf[ptr++] = (v % 10) + '0';
            v /= 10;
        } while (v);
        while (ptr) {
            put(buf[--ptr]);
        }
    }
    return *this;
}

Stream &Stream::operator<<(uint32_t v) {
    if (v >> 16) {
        *this << uint16_t(v >> 16);
    }
    return *this << uint16_t(v);
}

Stream &Stream::operator<<(int16_t v) {
    if (v < 0) {
        put('-');
        v = -v; // -65536 causes error; so does below
    }
    return *this << uint16_t(v);
}

Stream &Stream::operator<<(int32_t v) {
    if (v < 0) {
        put('-');
        v = -v;
    }
    return *this << uint32_t(v);
}

Stream &Stream::operator<<(Stream &(*f)(Stream &)) {
    return f(*this);
}

Stream &endl(Stream &t) {
    return t << '\n';
}

Stream &hex(Stream &t) {
    t.setf(t.getf() | Stream::HEX);
    return t;
}

Stream &dec(Stream &t) {
    t.setf(t.getf() & (~Stream::HEX));
    return t;
}