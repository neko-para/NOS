#include "io/term.h"
#include "boot/gdt.h"
#include "util/debug.h"

extern "C" void kernel_main() {
    Gdt::init();
    Term::init();
    QemuDebug::init();

    term() << "Gdt set! Still printing well." << endl;
    debug() << "From debug()!" << endl;
}