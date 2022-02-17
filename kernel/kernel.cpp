#include "term.h"
#include "boot/gdt.h"

extern "C" void kernel_main() {
    Gdt::init();
    Term::init();

    *term << "Gdt set! Still printing well." << endl;
}