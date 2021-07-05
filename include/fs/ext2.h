#ifndef _EXT2_H_
#define _EXT2_H_
/**
 * EXT2 implementation of VFS interface
 **/

#include "lib/stdint.h"
#include "kernel/device.h"
#include "io/streams.h"
#include "fs/fs.h"

typedef struct __attribute__((__packed__)){
    uint32_t block_bitmap;
    uint32_t inode_bitmap;
    uint32_t inode_table;
    uint16_t free_blocks_count;
    uint16_t free_inodes_count;
    uint16_t used_dirs_count;
    uint8_t reserved[14];
} Ext2BlockGroupDescriptor;

#define EXT2_INODE_TYPE_FIFO        0x1
#define EXT2_INODE_TYPE_CHARDEV     0x2
#define EXT2_INODE_TYPE_DIRECTORY   0x4
#define EXT2_INODE_TYPE_BLOCKDEV    0x6
#define EXT2_INODE_TYPE_FILE        0x8
#define EXT2_INODE_TYPE_SYMLINK     0xa
#define EXT2_INODE_TYPE_SOCKET      0xc

#define EXT2_INODE_ROOT_DIR         2


typedef struct __attribute__((__packed__)){
    Inode inode;
    uint32_t osd1;
    uint32_t blocks[15];
    uint32_t generation;
    uint32_t file_acl;
    uint32_t dir_acl;
    uint32_t faddr;
    uint8_t osd2[12];
} Ext2Inode;

#define E2INODE(i)  ((Ext2Inode*)(i))

typedef struct __attribute__((__packed__)){
    uint32_t inode_count;
    uint32_t block_count;
    uint32_t r_block_count;
    uint32_t free_block_count;
    uint32_t free_inode_count;
    uint32_t first_data_block;
    uint32_t log_block_size;
    uint32_t log_frag_size;
    uint32_t blocks_per_group;
    uint32_t frags_per_group;
    uint32_t inodes_per_group;
    uint32_t mtime;
    uint32_t wtime;
    uint16_t mount_count;
    uint16_t max_mount_count;
    uint16_t magic;
    uint16_t state;
    uint16_t errors;
    uint16_t minor_rev_level;
    uint32_t last_check;
    uint32_t check_interval;
    uint32_t creator_os;
    uint32_t rev_level;
    uint16_t def_resuid;
    uint16_t def_resgid;

    uint32_t first_inode;
    uint16_t inode_size;
    uint16_t block_group_nr;
    uint32_t feat_compat;
    uint32_t feat_incompat;
    uint32_t feat_ro_compat;
    uint8_t uuid[16];
    uint8_t volume_name[16];
    uint8_t last_mounted[64];
    uint32_t algo_bitmap;

    uint8_t prealloc_blocks;
    uint8_t prealloc_dir_blocks;
    uint16_t alignment;

    uint8_t journal_uuid[16];
    uint32_t journal_inum;
    uint32_t journal_dev;
    uint32_t last_orphan;

    uint32_t hash_seed[4];
    uint8_t hash_version;
    uint8_t unused1[3];

    uint32_t default_mount_options;
    uint32_t first_meta_bg;
    uint8_t unused2[760];
} Ext2Superblock;

typedef struct {
    uint32_t disk_pos;
    uint32_t pos;
} BlockPtr;

#define EXT2_DIR_ENTRY_UNKNOWN  0
#define EXT2_DIR_ENTRY_FILE     1
#define EXT2_DIR_ENTRY_DIR      2
#define EXT2_DIR_ENTRY_CHARDEV  3
#define EXT2_DIR_ENTRY_BLOCKDEV 4
#define EXT2_DIR_ENTRY_FIFO     5
#define EXT2_DIR_ENTRY_SOCKET   6
#define EXT2_DIR_ENTRY_SYMLINK  7

#define FS_MODE_R               0
#define FS_MODE_W               1
#define FS_MODE_RW              2
#define FS_MODE_WA              3
#define FS_MODE_RWA             4

void            ext2_register_type      (void);
#endif
