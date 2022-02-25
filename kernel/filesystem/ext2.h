#pragma once

#include <stdint.h>
#include "../vfs/vfs.h"
#include "../util/new.h"

class EXT2 : public VFS::FileSystem {
public:
    struct SuperBlock {
        uint32_t inodes_count;
        uint32_t blocks_count;
        uint32_t su_reserve;
        uint32_t free_blocks;
        uint32_t free_inodes;
        uint32_t first_data;
        uint32_t log_block_size;
        uint32_t log_frag_size;
        uint32_t blocks_per_group;
        uint32_t flags_per_group;
        uint32_t inodes_per_group;
        uint32_t last_mount;
        uint32_t last_written;
        uint16_t mount_times_since_check;
        uint16_t mount_times_allowed_before_check;
        uint16_t signature;
        uint16_t state;
        uint16_t error_handle_method;
        uint16_t minor_version;
        uint32_t time_since_check;
        uint32_t time_forced_check;
        uint32_t os_created_id;
        uint32_t major_version;
        uint16_t blocks_for_userid;
        uint16_t blocks_for_groupid;

        /** major_version >= 1 **/
        uint32_t first_inode;
        uint16_t inode_size;
        uint16_t block_group;
        uint32_t optional_feature;
        uint32_t required_feature;
        uint32_t readonly_feature;
        uint8_t filesystem_id[16];
        char volumn_name[16];
        char last_mounted_path[64];
        uint32_t compress_algo;
        uint8_t file_blocks_preallocate_count;
        uint8_t dir_blocks_preallocate_count;
        uint16_t padding;
        uint8_t journal_id[16];
        uint32_t journal_inode;
        uint32_t journal_device;
        uint32_t orphan_inode_list_head;

        uint32_t blockGroupCount() const {
            return (inodes_count + inodes_per_group - 1) / inodes_per_group;
        }

        void locateInode(uint32_t i, uint32_t &group, uint32_t &block, uint32_t &inner) const;
    };

    struct BlockGroupDescriptor {
        uint32_t block_usage_address;
        uint32_t inode_usage_address;
        uint32_t inode_table_address;
        uint16_t free_blocks_count;
        uint16_t free_inodes_count;
        uint16_t directory_count;
        // uint8_t padding[14];
    } __attribute__((packed));

    struct Inode {
        enum {
            T_FIFO = 1 << 12,
            T_CHAR_DEV = 2 << 12,
            T_DIRECTORY = 4 << 12,
            T_BLOCK_DEV = 6 << 12,
            T_REGULAR_FILE = 8 << 12,
            T_SYMLINK = 10 << 12,
            T_UNIX_SOCKET = 12 << 12,
            T_MASK = 15 << 12
        };
        uint16_t type_permissions;
        uint16_t user_id;
        uint32_t size_lo;
        uint32_t last_access_time;
        uint32_t creation_time;
        uint32_t last_modification_time;
        uint32_t deletion_time;
        uint16_t group_id;
        uint16_t hard_link_count;
        uint32_t sector_count; // without inode
        uint32_t flags;
        uint32_t os_spec_value1;
        uint32_t direct_block_pointer[12];
        uint32_t singly_indirect_block_pointer;
        uint32_t doubly_indirect_block_pointer;
        uint32_t triply_indirect_block_pointer;
        uint32_t generation_number;
        uint32_t file_acl;
        uint32_t directory_acl; // size_hi
        uint32_t fragment_block_address;
        uint8_t os_spec_value[12];
    };

    struct DirectoryEntry {
        uint32_t inode;
        uint16_t size;
        uint8_t length_lo;
        uint8_t type; // length_hi
        char name[0];

        DirectoryEntry *next() {
            return reinterpret_cast<DirectoryEntry *>(reinterpret_cast<uint8_t *>(this) + size);
        }
    };

    struct VFSFileDescriptor : public VFS::FileDescriptor {
        uint32_t seekg = 0;

        virtual VFSFileDescriptor *clone() const {
            return new VFSFileDescriptor(*this);
        }
        int32_t read(void *buf, uint32_t size) override;
    };

    struct VFSFile : public VFS::RegularFile {
        EXT2 *fs;
        uint32_t id;
        EXT2::Inode *inode;

        VFSFile(EXT2 *fs, uint32_t i, EXT2::Inode *n = nullptr);

        ~VFSFile() {
            delete inode;
        }
        EXT2::VFSFileDescriptor *open(int32_t flag) override;
        int32_t stat(Stat *buf) override;
    };

    struct VFSDirectory : public VFS::Directory {
        EXT2 *fs;
        uint32_t id;
        EXT2::Inode *inode;

        VFSDirectory(EXT2 *fs, uint32_t i, EXT2::Inode *n = nullptr);

        ~VFSDirectory() {
            delete inode;
        }
    };

    EXT2(uint32_t drive, uint32_t start, uint32_t size);

    void readBlock(uint32_t i, void *data);
    void *readBlock(uint32_t i);
    Inode *readInode(uint32_t i);
    void readInodeContent(Inode *inode, void *data); // no ext2 in inodes, so have to be called here
    void *readInodeContent(Inode *inode);

    VFS::FilePtr root() override;

private:
    uint32_t drive, base, length;
    SuperBlock super;
    BlockGroupDescriptor *desc;
    VFS::FilePtr _root;
};