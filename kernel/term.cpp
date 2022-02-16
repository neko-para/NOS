#include "string.h"
#include "term.h"
#include "new.h"

static uint8_t term_data[sizeof (Term)];
Term *term;

void Term::init() {
    term = new (term_data)Term(reinterpret_cast<uint16_t *>(0xB8000), 80, 25);
    term->clear(DEFAULT_ENTRY);
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
    ;
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
}