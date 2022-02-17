#include "term.h"

extern "C" void kernel_main() {
    Term::init();

    *term << "row" << 1 << endl
        << "row" << hex << 0xFE << endl
        << "Hello world!" << endl;
    term->scroll(1);
}