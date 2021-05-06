#ifndef _EXT2_H_
#define _EXT2_H_

#include "stdint.h"
#include "device.h"

typedef struct {
    uint32_t block_bitmap;
    uint32_t inode_bitmap;
    uint32_t inode_table;
    uint16_t free_blocks_count;
    uint16_t free_inodes_count;
    uint16_t used_dirs_count;
    uint16_t pad;
    uint8_t reserved[12];
} Ext2BlockGroupDescriptor;

typedef struct {
    uint16_t mode;
    uint16_t uid;
    uint32_t size;
    uint32_t atime;
    uint32_t ctime;
    uint32_t mtime;
    uint32_t dtime;
    uint16_t gid;
    uint16_t link_count;
    uint32_t block_count;
    uint32_t flags;
    uint32_t osd1;
    uint32_t blocks[15];
    uint32_t generation;
    uint32_t file_acl;
    uint32_t dir_acl;
    uint32_t faddr;
    uint8_t osd2[12];
} Ext2Inode;

typedef struct {
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

typedef struct Ext2Dirent {
    uint32_t inode;
    uint16_t rec_len;
    uint8_t name_len;
    uint8_t file_type;
    char name[1];
} Ext2Dirent;

typedef struct {
    BlockDevice* device;
    Ext2Superblock super_block;
    uint32_t block_size;
    uint32_t first_block_group_pos;
} Ext2FileSystem;

typedef int8_t (*ListIterator)(Ext2FileSystem*, Dirent*, void*);

int16_t ext2_open   (Ext2FileSystem* fs, BlockDevice* device);
void    ext2_list   (Ext2FileSystem* fs, const char* path, ListIterator iterator, void* func_data);
void    ext2_close  (Ext2FileSystem* fs);


#endif
