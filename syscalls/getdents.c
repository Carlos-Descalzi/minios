//#define NODEBUG
#include "kernel/syscalls.h"
#include "lib/stdint.h"
#include "kernel/isr.h"
#include "misc/debug.h"
#include "kernel/task.h"
#include "kernel/device.h"
#include "fs/fs.h"
#include "fs/ext2.h"

void syscall_getdents(InterruptFrame* f){
    Ext2Inode inode;
    struct {
        uint16_t device_id;
        uint32_t offset_next;
        Ext2DirEntry direntry;
    } * getdent_data = tasks_to_kernel_address((void*)f->ebx);

    debug("device id:");debug_i(getdent_data->device_id,16);debug("\n");
    debug("offset:");debug_i(getdent_data->offset_next,10);debug("\n");

    Device* device = device_find_by_id(getdent_data->device_id);

    if (!device){
        f->ebx = (uint32_t)-1;
        debug("11\n");
        return;
    }

    Ext2FileSystem* fs = ext2_open(BLOCK_DEVICE(device));

    if (!fs){
        debug(">12\n");
        f->ebx = (uint32_t)-2;
        return;
    }

    if (ext2_load_inode(fs, getdent_data->direntry.inode, &inode)){
        f->ebx = (uint32_t)-3;
        debug(">13\n");
        return;
    }

    f->ebx = ext2_get_direntry(fs, &inode, &(getdent_data->offset_next), &(getdent_data->direntry));

    debug("getdents:");debug(getdent_data->direntry.name);debug("\n");

    ext2_close(fs);
    debug("done.\n");
}
