#include "kernel/syscalls.h"
#include "kernel/task.h"
#include "fs/ext2.h"
#include "lib/path.h"
#include "misc/debug.h"

struct stat {
    uint16_t    st_dev;
    uint32_t    st_ino;
    uint32_t    st_mode;
    uint32_t    st_nlink;
    uint32_t    st_uid;
    uint32_t    st_gid;
    uint16_t    st_rdev;
    uint32_t    st_size;
    uint32_t    st_blksize;
    uint32_t    st_blocks;
    uint32_t    st_atim;
    uint32_t    st_mtim;
    uint32_t    st_ctim;
};

void syscall_stat(InterruptFrame* f){
    uint16_t device_id;
    char filepath[256];
    uint32_t inode_num;
    Ext2Inode inode;

    struct {
        char* pathname;
        struct stat* statbuf;
    } *stat_data = tasks_to_kernel_address((void*)f->ebx);

    char* pathname = stat_data->pathname;
    struct stat* statbuf = stat_data->statbuf;

    pathname = tasks_to_kernel_address(pathname);

    if (path_parse(pathname, &device_id, filepath)){
        f->ebx = ((uint32_t)-1);
        debug("1\n");
        return;
    }

    Device* device = device_find_by_id(device_id);

    if (!device){
        f->ebx = ((uint32_t)-2);
        return;
    }

    Ext2FileSystem* fs = ext2_open(BLOCK_DEVICE(device));

    if(!fs){
        f->ebx = ((uint32_t)-3);
        return;
    }

    inode_num = ext2_find_inode(fs, filepath);

    if (inode_num){

        ext2_load_inode(fs, inode_num, &inode);
        statbuf = tasks_to_kernel_address(statbuf);
        statbuf->st_dev = device_id;
        statbuf->st_ino = inode_num;
        statbuf->st_uid = inode.uid;
        statbuf->st_gid = inode.gid;
        statbuf->st_nlink = inode.link_count;
        statbuf->st_blocks = inode.block_count;
        statbuf->st_blksize = fs->block_size;
        statbuf->st_mode = inode.mode;
        statbuf->st_size = inode.size;
        statbuf->st_atim = inode.atime;
        statbuf->st_mtim = inode.mtime;
        statbuf->st_ctim = inode.ctime;

        f->ebx = 0;

    } else {
        f->ebx = ((uint32_t)-4);
    }

    ext2_close(fs);
}
