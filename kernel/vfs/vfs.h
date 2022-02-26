#pragma once

#include <stdint.h>
#include "../export/nos.h"
#include "../util/array.h"
#include "../util/string.h"

namespace VFS {

struct File;
struct FileDescriptor;
struct RegularFile;
struct Directory;
struct CharacterDevice;
struct SymLink;
struct FileSystem;

struct File {
    enum {
        T_FIFO = 1 << 12,
        T_CHAR_DEV = 2 << 12,
        T_DIRECTORY = 4 << 12,
        T_BLOCK_DEV = 6 << 12,
        T_REGULAR_FILE = 8 << 12,
        T_SYMLINK = 10 << 12,
        T_UNIX_SOCKET = 12 << 12,
        T_MASK = 15 << 12,

        A_NEED_DELETE = 1
    };

    uint16_t type;
    uint8_t attrib;

    File() {
        attrib = 0;
    }
    virtual ~File() {}
    virtual int32_t read(void *, uint32_t ) {
        return -1;
    }
    virtual int32_t write(const void *, uint32_t ) {
        return -1;
    }
    virtual int32_t stat(Stat *) {
        return -1;
    }
    virtual FileDescriptor *open(int32_t );
};

struct FilePtr {
    struct FilePtrData {
        File *file;
        int32_t refer;
    };

    FilePtr(File *file = 0) {
        if (!file) {
            data = 0;
        } else if (file->attrib & File::A_NEED_DELETE) {
            data = new FilePtrData { file, 1 };
        } else {
            data = new FilePtrData { file, -1 };
        }
    }
    FilePtr(const FilePtr &fpd) {
        data = fpd.data;
        if (data) {
            if (data->refer > 0) {
                data->refer++;
            } else {
                data->refer--;
            }
        }
    }
    ~FilePtr() {
        if (data) {
            if (data->refer > 0) {
                if (--data->refer == 0) {
                    delete data->file;
                    delete data;
                }
            } else {
                if (++data->refer == 0) {
                    delete data;
                }
            }
        }
    }
    FilePtr &operator=(const FilePtr &fpd) {
        if (data == fpd.data) {
            return *this;
        }
        if (data) {
            if (data->refer > 0) {
                if (--data->refer == 0) {
                    delete data->file;
                    delete data;
                }
            } else {
                if (++data->refer == 0) {
                    delete data;
                }
            }
        }
        data = fpd.data;
        if (data) {
            if (data->refer > 0) {
                data->refer++;
            } else {
                data->refer--;
            }
        }
        return *this;
    }

    File *get() const {
        return data ? data->file : 0;
    }

    operator bool() const {
        return data && data->file;
    }

    RegularFile *getRegularFile() const;
    Directory *getDirectory() const;
    SymLink *getSymLink() const;

    File *operator->() const {
        return get();
    }

    FileDescriptor *open(const String &path, int32_t flag) const;

    FilePtrData *data;
};

void init();
FilePtr lookup(const String &path);
void mount(const String &path, FileSystem *filesystem);

struct FileSystem {
    virtual ~FileSystem() {}
    virtual FilePtr root() {
        return 0;
    }
};

struct FileDescriptor {
    FilePtr file;
    String path;

    virtual ~FileDescriptor() {}
    virtual FileDescriptor *clone() const {
        return new FileDescriptor(*this);
    }
    virtual int32_t read(void *buf, uint32_t size) {
        return file ? file->read(buf, size) : -1;
    }
    virtual int32_t write(const void *buf, uint32_t size) {
        return file ? file->write(buf, size) : -1;
    }
    virtual int32_t close();
};

inline FileDescriptor *FilePtr::open(const String &path, int32_t flag) const {
    if (data) {
        auto *fd = data->file->open(flag);
        fd->file = *this;
        fd->path = path;
        return fd;
    } else {
        return 0;
    }
}

struct CharacterDevice : public File {
    CharacterDevice() {
        type = T_CHAR_DEV;
    }

    virtual int32_t read(uint8_t &) {
        return 0;
    }
    virtual int32_t write(uint8_t ) {
        return 0;
    }
    virtual int32_t read(void *buf, uint32_t size) {
        uint8_t *p = reinterpret_cast<uint8_t *>(buf);
        int32_t rd = 0;
        while (size--) {
            if (read(*p++)) {
                return rd;
            }
            rd++;
        }
        return rd;
    }
    virtual int32_t write(const void *buf, uint32_t size) {
        const uint8_t *p = reinterpret_cast<const uint8_t *>(buf);
        int32_t wt = 0;
        while (size--) {
            if (write(*p++)) {
                return wt;
            }
            wt++;
        }
        return wt;
    }
};

struct RegularFile : public File {
    RegularFile() {
        type = T_REGULAR_FILE;
    }
};

struct Directory : public File {
    struct Entry {
        String name;
        FilePtr file;
    };

    FileSystem *mount;
    Array<Entry> sub;

    Directory() {
        type = T_DIRECTORY;
        mount = 0;
    }
    virtual FilePtr find(const String &name) {
        if (mount) {
            return mount->root().getDirectory()->find(name);
        }
        for (uint32_t i = 0; i < sub.size(); i++) {
            if (sub[i].name == name) {
                return sub[i].file;
            }
        }
        return 0;
    }
    void append(const String &path, FilePtr file) {
        if (mount) {
            mount->root().getDirectory()->append(path, file);
        } else {
            sub.pushBack(Entry { path, file });
        }
    }
};

struct SymLink : public File {
    String target;

    SymLink(const String &t) : target(t) {
        type = T_SYMLINK;
    }
    
    virtual FileDescriptor *open(int32_t flag) {
        FilePtr f = lookup(target);
        return f.open(target, flag);
    }
};

inline RegularFile *FilePtr::getRegularFile() const {
    File *f = get();
    if (f && (f->type & File::T_MASK) == File::T_REGULAR_FILE) {
        return static_cast<RegularFile *>(f);
    }
    return 0;
}

inline Directory *FilePtr::getDirectory() const {
    File *f = get();
    if (f && (f->type & File::T_MASK) == File::T_DIRECTORY) {
        return static_cast<Directory *>(f);
    }
    return 0;
}

inline SymLink *FilePtr::getSymLink() const {
    File *f = get();
    if (f && (f->type & File::T_MASK) == File::T_SYMLINK) {
        return static_cast<SymLink *>(f);
    }
    return 0;
}

};