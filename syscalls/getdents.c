#define NODEBUG
#include "kernel/syscalls.h"
#include "lib/stdint.h"
#include "lib/string.h"
#include "kernel/isr.h"
#include "misc/debug.h"
#include "kernel/task.h"
#include "kernel/device.h"
#include "fs/fs.h"

void syscall_getdents(InterruptFrame* f){
    struct {
        uint16_t device_id;
        uint32_t offset_next;
        DirEntry direntry;
    } * getdent_data = tasks_to_kernel_address((void*)f->ebx);

    Device* device = device_find_by_id(getdent_data->device_id);

    if (!device){
        f->ebx = (uint32_t)-1;
        return;
    }

    FileSystem* fs = fs_get_filesystem(BLOCK_DEVICE(device));
    Inode* inode = fs_alloc_inode(fs);

    if (!fs){
        f->ebx = (uint32_t)-2;
        return;
    }

    if (fs_load_inode(fs, getdent_data->direntry.inode, inode)){
        f->ebx = (uint32_t)-3;
        return;
    }
    memset(&(getdent_data->direntry),0,sizeof(DirEntry));

    f->ebx = fs_get_direntry(fs, inode, &(getdent_data->offset_next), &(getdent_data->direntry));

    fs_free_inode(fs, inode);
    fs_release_filesystem(fs);
}
