#include "ext2.h"
#include "../io/disk.h"
#include "../util/memory.h"
#include "../util/new.h"
#include "../util/string.h"
#include "../io/term.h"

void EXT2::SuperBlock::locateInode(uint32_t i, uint32_t &group, uint32_t &block, uint32_t &inner) const {
    group = (i - 1) / inodes_per_group;
    uint32_t index = (i - 1) % inodes_per_group;
    block = (index * inode_size) / 1024;
    inner = (index * inode_size) % 1024;
}

int32_t EXT2::VFSFileDescriptor::read(void *buf, uint32_t size) {
    auto *f = reinterpret_cast<VFSFile *>(file.getRegularFile());
    if (seekg == f->inode->size_lo) {
        return 0;
    }
    if (seekg + size >= f->inode->size_lo) {
        size = f->inode->size_lo - seekg;
    }
    uint8_t *content = reinterpret_cast<uint8_t *>(f->fs->readInodeContent(f->inode));
    memcpy(buf, content + seekg, size);
    Memory::free(content);
    return size;
}

int32_t EXT2::VFSFileDescriptor::seek(int32_t offset, int32_t whence) {
    auto *f = reinterpret_cast<VFSFile *>(file.getRegularFile());
    switch (whence) {
    case SEEK_SET:
        if (offset < 0) {
            return -1;
        }
        seekg = offset;
        break;
    case SEEK_CUR:
        if (int32_t(seekg) + offset < 0) {
            return -1;
        }
        seekg += offset;
        break;
    case SEEK_END:
        if (int32_t(f->inode->size_lo) + offset < 0) {
            return -1;
        }
        seekg = f->inode->size_lo + offset;
        break;
    default:
        return -1;
    }
    return seekg;
}


EXT2::VFSFile::VFSFile(EXT2 *fs, uint32_t i, EXT2::Inode *n) : fs(fs), id(i), inode(n) {
    if (!inode) {
        inode = fs->readInode(i);
    }
}

EXT2::VFSFileDescriptor *EXT2::VFSFile::open(int32_t ) {
    return new VFSFileDescriptor();
}

int32_t EXT2::VFSFile::stat(Stat *buf) {
    buf->size = inode->size_lo;
    return 0;
}

EXT2::VFSDirectory::VFSDirectory(EXT2 *fs, uint32_t i, EXT2::Inode *n) : fs(fs), id(i), inode(n) {
    if (!inode) {
        inode = fs->readInode(i);
    }
    uint32_t sz = 0;
    void *content = fs->readInodeContent(inode);
    EXT2::DirectoryEntry *entry = reinterpret_cast<EXT2::DirectoryEntry *>(content);
    while (sz < inode->size_lo) {
        char *buf = new char[entry->length_lo + 1];
        memcpy(buf, entry->name, entry->length_lo);
        buf[entry->length_lo] = 0;
        if (buf[0] == '.' && ((buf[1] == '.' && buf[2] == 0) || buf[1] == 0)) {
            delete[] buf;
            sz += entry->size;
            entry = entry->next();
            continue;
        }
        uint32_t i = entry->inode;
        auto in = fs->readInode(i);
        switch (in->type_permissions & EXT2::Inode::T_MASK) {
            case EXT2::Inode::T_REGULAR_FILE:
                sub.pushBack(Entry { buf, new VFSFile(fs, i, in) });
                break;
            case EXT2::Inode::T_DIRECTORY:
                sub.pushBack(Entry { buf, new VFSDirectory(fs, i, in) });
                break;
            default:
                delete in;
        }
        delete[] buf;
        sz += entry->size;
        entry = entry->next();
    }
    Memory::free(content);
}

EXT2::EXT2(uint32_t drive, uint32_t start, uint32_t size) : drive(drive), base(start), length(size) {
    Disk::command(drive, 1, base + 2, Disk::C_READ);
    auto sec = Disk::read();
    memcpy(&super, sec, sizeof (SuperBlock));

    uint32_t nBlockGroup = super.blockGroupCount();
    Disk::command(drive, 1, base + 4, Disk::C_READ);
    Disk::read(sec);
    desc = new BlockGroupDescriptor[nBlockGroup];
    for (uint32_t i = 0; i < nBlockGroup; i++) {
        memcpy(desc + i, sec + i * 16, sizeof (BlockGroupDescriptor));
    }
    Memory::free(sec);
    _root = VFS::FilePtr(new VFSDirectory(this, 2));
}

void EXT2::readBlock(uint32_t i, void *data) {
    Disk::command(drive, 2, base + i * 2, Disk::C_READ);
    Disk::read(reinterpret_cast<uint16_t *>(data));
}

void *EXT2::readBlock(uint32_t i) {
    void *buf = Memory::alloc(1024);
    readBlock(i, buf);
    return buf;
}

EXT2::Inode *EXT2::readInode(uint32_t i) {
    Inode *inode = new Inode;
    uint32_t group, block, inner;
    super.locateInode(i, group, block, inner);
    void *inodes = readBlock(desc[group].inode_table_address + block);
    memcpy(inode, reinterpret_cast<uint8_t *>(inodes) + inner, sizeof (Inode));
    Memory::free(inodes);
    return inode;
}

void EXT2::readInodeContent(Inode *inode, void *data) {
    uint8_t *ptr = reinterpret_cast<uint8_t *>(data);
    uint32_t size = inode->size_lo;
    for (int i = 0; i < 12; i++) {
        if (inode->direct_block_pointer[i] != 0) {
            if (size > 1024) {
                size -= 1024;
                readBlock(inode->direct_block_pointer[i], ptr);
                ptr += 1024;
            } else {
                void *buf = readBlock(inode->direct_block_pointer[i]);
                memcpy(ptr, buf, size);
                Memory::free(buf);
                return;
            }
        }
    }
    if (inode->singly_indirect_block_pointer != 0) {
        uint32_t *pt = reinterpret_cast<uint32_t *>(readBlock(inode->singly_indirect_block_pointer));
        for (int i = 0; i < 256; i++) {
            if (pt[i] != 0) {
                if (size > 1024) {
                    size -= 1024;
                    readBlock(pt[i], ptr);
                    ptr += 1024;
                } else {
                    void *buf = readBlock(pt[i]);
                    memcpy(ptr, buf, size);
                    Memory::free(buf);
                    Memory::free(pt);
                    return;
                }
            }
        }
    }
    // doubly and triply not supported yet
}

void *EXT2::readInodeContent(Inode *inode) {
    void *buf = Memory::alloc(inode->size_lo);
    readInodeContent(inode, buf);
    return buf;
}

VFS::FilePtr EXT2::root() {
    return _root;
}
