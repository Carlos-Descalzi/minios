#include "kernel/syscall.h"
#include "kernel/modules.h"
#include "kernel/task.h"
#include "lib/string.h"
#include "fs/fs.h"
#include "lib/path.h"
#include "kernel/device.h"
#include "misc/debug.h"

void syscall_modload(InterruptFrame* f){
    char path[256];

    char* full_path = tasks_to_kernel_address((void*)f->ebx);

    uint16_t device_id;

    if (path_parse(full_path, &device_id, path)){
        debug("modload - bad path\n");
        f->ebx = (uint32_t)-1;
        return;
    }

    Device* device = device_find_by_id(device_id);

    if (!device){
        debug("modload - no device\n");
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
