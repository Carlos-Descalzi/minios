#include "kernel/syscall.h"
#include "kernel/modules.h"
#include "kernel/task.h"
#include "lib/string.h"
#include "fs/fs.h"
#include "kernel/device.h"

void syscall_modload(InterruptFrame* f){
    char devname[20];
    char* pos;

    char* path = tasks_to_kernel_address((void*)f->ebx);

    pos = strchr(path,':');
    strncpy(devname, path, ((uint32_t)pos)-((uint32_t)path));

    if (!strlen(devname)){
        // no device in path
        f->ebx = ((uint32_t)-1);
    }

    Device* device = device_find_by_name(devname);

    if (!device){
        // unknown device
        f->ebx = ((uint32_t)-2);
        return;
    } 
    FileSystem* fs = fs_get_filesystem(BLOCK_DEVICE(device));

    if (fs){
        int32_t result = modules_load(fs, path);

        f->ebx = ((uint32_t) 10 * result);

        fs_release_filesystem(fs);
    } else {
        // no fs
        f->ebx = ((uint32_t)-3);
    }

}
