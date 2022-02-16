#include "term.h"

extern "C" void kernel_main() {
    Term::init();

    term->puts("row1\nrow2\nHello world!\n");
    term->scroll(1);
}