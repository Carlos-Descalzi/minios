#define NODEBUG
#include "kernel/syscalls.h"
#include "kernel/task.h"
#include "kernel/common.h"
#include "fs/fs.h"
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

typedef struct {
    char* pathname;
    struct stat* statbuf;
} StatData;

uint32_t syscall_stat(SyscallArg arg){
    char filepath[PATH_SIZE];
    uint16_t device_id;
    uint32_t inode_num;
    Inode* inode;

    StatData* stat_data = tasks_to_kernel_address(arg.ptr_arg, sizeof(StatData));

    char* pathname = stat_data->pathname;
    struct stat* statbuf = stat_data->statbuf;

    debug("Stat:");

    pathname = tasks_to_kernel_address(pathname, PATH_SIZE);
    debug(pathname);
    debug("\n");

    int parse_result = path_parse(pathname, &device_id, filepath);

    if (parse_result){
        debug("stat - unable to parse path: ");debug(pathname);debug("\n");
        return (uint32_t) -1;
    }

    Device* device = device_find_by_id(device_id);

    if (!device){
        debug("stat - unable to find device ");debug_i(device_id,16);debug("\n");
        return (uint32_t) -2;
    }

    FileSystem* fs = fs_get_filesystem(BLOCK_DEVICE(device));

    if(!fs){
        debug("stat - unable to get filesystem for device\n");
        return (uint32_t) -3;
    }

    inode_num = fs_find_inode(fs, filepath);

    uint32_t result = 0;

    if (inode_num){

        inode = fs_alloc_inode(fs);

        fs_load_inode(fs, inode_num, inode);
        statbuf = tasks_to_kernel_address(statbuf, sizeof(struct stat));
        statbuf->st_dev = device_id;
        statbuf->st_ino = inode_num;
        statbuf->st_uid = inode->uid;
        statbuf->st_gid = inode->gid;
        statbuf->st_nlink = inode->link_count;
        statbuf->st_blocks = inode->block_count;
        statbuf->st_blksize = fs->block_size;
        statbuf->st_mode = inode->mode;
        statbuf->st_size = inode->size;
        statbuf->st_atim = inode->atime;
        statbuf->st_mtim = inode->mtime;
        statbuf->st_ctim = inode->ctime;

        fs_free_inode(fs, inode);

        result = 0;

    } else {
        debug("stat - inode not found for path\n");
        result = (uint32_t) -4;
    }

    fs_release_filesystem(fs);

    return result;
}
