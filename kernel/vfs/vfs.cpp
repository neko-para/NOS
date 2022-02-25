#include "vfs.h"

namespace VFS {

int32_t FileDescriptor::close() {
    return 0;
}

FileDescriptor *File::open(int32_t flag) {
    return new FileDescriptor();
}

static Directory *root;

void init() {
    root = new Directory();
}

FilePtr lookup(const String &path) {
    int32_t pos;
    int32_t prev = 0;
    FilePtr d(root);
    if (path.length() == 0 || path[0] != '/') {
        return 0;
    } else if (path == "/") {
        return root;
    }
    do {
        pos = path.indexOf('/', prev + 1);
        if (pos == -1) {
            return d.getDirectory()->find(path.substr(prev + 1));
        } else {
            d = d.getDirectory()->find(path.substr(prev + 1, pos - prev - 1));
            prev = pos;
        }
    } while (d);
}

void mount(const String &path, FileSystem *filesystem) {
    FilePtr f = lookup(path);
    if (f) {
        Directory *d = f.getDirectory();
        if (d) {
            if (d->mount) {
                delete d->mount;
            }
            d->mount = filesystem;
        }
    }
}

}
