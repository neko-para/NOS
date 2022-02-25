#pragma once

#include "../vfs/vfs.h"

struct ProcFS : public VFS::FileSystem {

    struct FDInfo : public VFS::Directory {
        FDInfo(uint32_t id) : pid(id) {}
        VFS::FilePtr find(const String &path) override;

        uint32_t pid;
    };

    struct ProcessInfo : public VFS::Directory {
        ProcessInfo(uint32_t id) : pid(id) {}
        VFS::FilePtr find(const String &path) override;

        uint32_t pid;
    };

    struct Directory : public VFS::Directory {
        VFS::FilePtr find(const String &path) override;
    };

    ProcFS();
    VFS::FilePtr root() override {
        return _root;
    }

    VFS::FilePtr _root;
};