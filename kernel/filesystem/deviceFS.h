#pragma once

#include "../vfs/vfs.h"

struct DeviceFS : public VFS::FileSystem {
    struct Termnial : public VFS::CharacterDevice {
        virtual int32_t read(uint8_t &data) override;
        virtual int32_t write(uint8_t data) override;
    };
    struct Directory : public VFS::Directory {
        Directory();
        VFS::FilePtr find(const String &path) override;

        Termnial *tty;
    };

    DeviceFS();
    VFS::FilePtr root() override;

    VFS::FilePtr _root;
};