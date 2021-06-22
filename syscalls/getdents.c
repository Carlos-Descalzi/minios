#define NODEBUG
#include "kernel/syscalls.h"
#include "lib/stdint.h"
#include "kernel/isr.h"
#include "misc/debug.h"
#include "kernel/task.h"
#include "kernel/device.h"
#include "fs/fs.h"

typedef struct {
    uint8_t device_kind;
    uint8_t device_instance;
    uint32_t offset_next;
    Ext2DirEntry direntry;
} UserDirent;

void syscall_getdents(InterruptFrame* f){
    /*
    Ext2Inode inode;

    struct {
        uint8_t device_kind;
        uint8_t device_instance;
        uint32_t offset_next;
        Ext2DirEntry direntry;
    } * getdent_data = tasks_to_kernel_address((void*)f->ebx);

    Device* device = device_find(getdent_data->device_kind, getdent_data->device_instance);

    Ext2FileSystem* fs = fs_get_filesystem(device);

    ext2_load_inode(fs, getdent_data->direntry.inode, &inode);

    f->ebx = ext2_get_direntry(fs, inode, getdent_data->offset_next, &(getdent_data->direntry));
    */
}
