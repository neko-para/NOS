#include "deviceFS.h"
#include "../io/keyboard.h"
#include "../io/term.h"

int32_t DeviceFS::Termnial::read(uint8_t &data) {
    while (true) {
        Keyboard::available->lock();
        KeyboardMessage msg;
        if (Keyboard::pop(msg)) {
            if (!(msg.flag & Keyboard::RELEASE) && msg.ch) {
                data = msg.ch;
                return 0;
            }
        }
    }
}

int32_t DeviceFS::Termnial::write(uint8_t data) {
    term().put(data);
    return 0;
}

DeviceFS::Directory::Directory() {
    tty = new DeviceFS::Termnial();
}

VFS::FilePtr DeviceFS::Directory::find(const String &path) {
    if (path == "tty") {
        return tty;
    } else if (path == "stdin") {
        auto *p = new VFS::SymLink("/proc/self/fd/0");
        p->attrib |= A_NEED_DELETE;
        return p;
    } else if (path == "stdout") {
        auto *p = new VFS::SymLink("/proc/self/fd/1");
        p->attrib |= A_NEED_DELETE;
        return p;
    }
    return 0;
}

DeviceFS::DeviceFS() : _root(new Directory()) {
}

VFS::FilePtr DeviceFS::root() {
    return _root;
}
