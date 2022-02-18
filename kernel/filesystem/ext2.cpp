#include "ext2.h"
#include "../io/disk.h"
#include "../util/memory.h"
#include "../util/new.h"
#include "../util/string.h"

void EXT2::SuperBlock::locateInode(uint32_t i, uint32_t &group, uint32_t &block, uint32_t &inner) const {
    group = (i - 1) / inodes_per_group;
    uint32_t index = (i - 1) % inodes_per_group;
    block = (index * inode_size) / 1024;
    inner = (index * inode_size) % 1024;
}


EXT2::Iterator::Iterator(EXT2 *ext2) : ext2(ext2) {
    this->ID = 2;
    this->inode = ext2->readInode(2);
}

EXT2::Iterator::~Iterator() {
    delete inode;
}

void EXT2::Iterator::enumDirectory(void (*func)(DirectoryEntry *entry)) {
    uint32_t sz = 0;
    void *content = ext2->readInodeContent(inode);
    EXT2::DirectoryEntry *entry = reinterpret_cast<EXT2::DirectoryEntry *>(content);
    while (sz < inode->size_lo) {
        func(entry);
        sz += entry->size;
        entry = entry->next();
    }
    Memory::free(content);
}

bool EXT2::Iterator::enterDirectory(const char *dir) {
    uint32_t sz = 0;
    uint32_t sl = strlen(dir);
    void *content = ext2->readInodeContent(inode);
    EXT2::DirectoryEntry *entry = reinterpret_cast<EXT2::DirectoryEntry *>(content);
    while (sz < inode->size_lo) {
        if (sl == entry->length_lo) {
            if (!memcmp(dir, entry->name, sl)) {
                if (entry->inode != ID) { // incase symlink to itself
                    ID = entry->inode;
                    delete inode;
                    inode = ext2->readInode(ID);
                    Memory::free(content);
                    return true;
                }
            }
        }
        sz += entry->size;
        entry = entry->next();
    }
    Memory::free(content);
    return false;
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

EXT2::Iterator *EXT2::root() {
    return new Iterator(this);
}