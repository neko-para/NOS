#include "procFS.h"
#include "../process/task.h"

VFS::FilePtr ProcFS::FDInfo::find(const String &path) {
    int32_t fd = path.toInt();
    const auto &fds = (*tasks[pid]->file);
    if (fd >= 0 && fd < fds.size()) {
        auto *p = new VFS::SymLink(fds[fd]->path);
        p->attrib |= A_NEED_DELETE;
        return p;
    } else {
        return 0;
    }
}

VFS::FilePtr ProcFS::ProcessInfo::find(const String &path) {
    if (path == "fd") {
        auto *p = new FDInfo(pid);
        p->attrib |= A_NEED_DELETE;
        return p;
    }
    return 0;
}

VFS::FilePtr ProcFS::Directory::find(const String &path) {
    int32_t pid = 0;
    if (path == "self") {
        pid = currentTask->tid;
    } else {
        pid = String(path).toInt();
    }
    auto *p = new ProcessInfo(pid);
    p->attrib |= A_NEED_DELETE;
    return p;
}

ProcFS::ProcFS() : _root(new Directory()) {
}
