#define NODEBUG
#include "kernel/syscalls.h"
#include "lib/stdint.h"
#include "lib/string.h"
#include "kernel/isr.h"
#include "misc/debug.h"
#include "kernel/task.h"
#include "kernel/device.h"
#include "fs/fs.h"

typedef struct {
    uint16_t device_id;
    uint32_t offset_next;
    DirEntry direntry;
} GetDentData;

uint32_t syscall_getdents(SyscallArg arg){
    GetDentData* getdent_data = tasks_to_kernel_address(arg.ptr_arg, sizeof(GetDentData));

    Device* device = device_find_by_id(getdent_data->device_id);

    if (!device){
        return (uint32_t) -1;
    }

    FileSystem* fs = fs_get_filesystem(BLOCK_DEVICE(device));
    Inode* inode = fs_alloc_inode(fs);

    if (!fs){
        return (uint32_t) -2;
    }

    if (fs_load_inode(fs, getdent_data->direntry.inode, inode)){
        return (uint32_t) -3;
    }
    memset(&(getdent_data->direntry),0,sizeof(DirEntry));

    uint32_t result = fs_get_direntry(fs, inode, &(getdent_data->offset_next), &(getdent_data->direntry));

    fs_free_inode(fs, inode);
    fs_release_filesystem(fs);

    return result;
}
