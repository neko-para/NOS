#pragma once

#include <stdint.h>
#include "stream.h"

inline uint8_t term_color(uint8_t fore, uint8_t back) {
    return fore | (back << 4);
}

inline uint16_t term_entry(uint8_t color, char ch) {
    return ch | ((uint16_t)color << 8);
}

class Term : public Stream {
public:
    enum Color {
        BLACK = 0,
        BLUE = 1,
        GREEN = 2,
        CYAN = 3,
        RED = 4,
        MAGENTA = 5,
        BROWN = 6,
        LIGHT_GREY = 7,
        DARK_GREY = 8,
        LIGHT_BLUE = 9,
        LIGHT_GREEN = 10,
        LIGHT_CYAN = 11,
        LIGHT_RED = 12,
        LIGHT_MAGENTA = 13,
        LIGHT_BROWN = 14,
        WHITE = 15,

        DEFAULT_ENTRY = 0x0720
    };

    static void init();

private:
    Term(uint16_t *buf, uint16_t w, uint16_t h);

public:
    void clear(uint16_t entry);

    virtual void put(char ch) override;

    void scroll(uint8_t nrow, uint16_t fill = DEFAULT_ENTRY);

    void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
    void disable_cursor();
    void update_cursor(uint8_t r, uint8_t c);

private:
    uint16_t *buffer;
    uint8_t row = 0, column = 0;
    uint8_t width, height;
    uint8_t attrib = DEFAULT_ENTRY >> 8;
};

Term &term();