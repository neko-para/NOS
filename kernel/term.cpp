#include "io.h"
#include "new.h"
#include "string.h"
#include "term.h"

const uint16_t hexFlag = 0x0001;

static uint8_t term_data[sizeof (Term)];
Term *term;

void Term::init() {
    term = new (term_data)Term(reinterpret_cast<uint16_t *>(0xB8000), 80, 25);
    term->clear(DEFAULT_ENTRY);
    term->enable_cursor(0, 15);
    term->update_cursor(0, 0);
}

Term::Term(uint16_t *buf, uint16_t w, uint16_t h) : buffer(buf), width(w), height(h) {
}

void Term::clear(uint16_t entry) {
    for (unsigned i = 0; i < width * height; i++) {
        buffer[i] = entry;
    }
}

void Term::putc(char ch) {
    if (ch == '\n') {
        goto _next_row;
    } else if (ch == '\r') {
        column = 0;
        return;
    } else if (ch == '\b') {
        column = column ? column - 1 : 0;
        goto _end;
    }
    buffer[row * width + column] = term_entry(attrib, ch);
    if (++column == width) {
_next_row:
        column = 0;
        ++row;
        if (row == height) {
            scroll(1);
        }
    }
_end:
    term->update_cursor(row, column);
}

void Term::puts(const char *str) {
    while (*str) {
        putc(*str++);
    }
}

void Term::scroll(uint8_t nrow, uint16_t fill) {
    if (nrow >= height) {
        clear(fill);
        row = 0;
    } else {
        memcpy(buffer, buffer + nrow * width, (height - nrow) * width * 2);
        for (unsigned i = (height - nrow) * width; i < width * height; i++) {
            buffer[i] = fill;
        }
        row -= nrow;
    }
    update_cursor(row, column);
}

void Term::enable_cursor(uint8_t cursor_start, uint8_t cursor_end) {
	outb(0x3D4, 0x0A);
	outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);
 
	outb(0x3D4, 0x0B);
	outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

void Term::disable_cursor() {
	outb(0x3D4, 0x0A);
	outb(0x3D5, 0x20);
}

void Term::update_cursor(uint8_t r, uint8_t c) {
    uint16_t pos = r * width + c;
 
	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t) (pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

inline char toHex(uint8_t n) {
    return n < 10 ? n + '0' : n + 'A' - 10;
}

Term &Term::operator<<(uint16_t v) {
    if (flag & hexFlag) {
        putc(toHex(v >> 12));
        putc(toHex((v >> 8) & 0xF));
        putc(toHex((v >> 4) & 0xF));
        putc(toHex(v & 0xF));
    } else {
        char buf[7];
        int ptr = 0;
        do {
            buf[ptr++] = (v % 10) + '0';
            v /= 10;
        } while (v);
        while (ptr) {
            putc(buf[--ptr]);
        }
    }
    return *this;
}

Term &Term::operator<<(uint32_t v) {
    return *this << uint16_t(v >> 16) << uint16_t(v);
}

Term &Term::operator<<(int16_t v) {
    if (v < 0) {
        putc('-');
        v = -v; // -65536 causes error; so does below
    }
    return *this << uint16_t(v);
}

Term &Term::operator<<(int32_t v) {
    if (v < 0) {
        putc('-');
        v = -v;
    }
    return *this << uint32_t(v);
}

Term &Term::operator<<(Term &(*f)(Term &)) {
    return f(*this);
}

Term &endl(Term &t) {
    return t << '\n';
}

Term &hex(Term &t) {
    t.setf(t.getf() | hexFlag);
    return t;
}

Term &dec(Term &t) {
    t.setf(t.getf() & (~hexFlag));
    return t;
}