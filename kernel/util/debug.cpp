#include "debug.h"
#include "new.h"
#include "../io/io.h"

static uint8_t debug_data[sizeof (QemuDebug)];
static QemuDebug *pdebug;

void QemuDebug::put(char ch) {
    outb(0x3F8, ch);
}

void QemuDebug::init() {
    pdebug = new (debug_data)QemuDebug;
}

QemuDebug &debug() {
    return *pdebug;
}