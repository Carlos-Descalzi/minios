#define NODEBUG
#include "kernel/syscalls.h"
#include "kernel/task.h"
#include "lib/path.h"
#include "kernel/device.h"
#include "fs/ext2.h"
#include "misc/debug.h"

void syscall_spawn(InterruptFrame* f){
    uint16_t device_id;
    char filepath[256];
    char* path = tasks_to_kernel_address((void*)f->ebx);

    debug("Spawning new task:");debug(path);debug("\n");

    if (path_parse(path, &device_id, filepath)){
        debug("Bad path\n");
        f->ebx = ((uint32_t)-1);
        return;
    }

    Device* device = device_find_by_id(device_id);

    if (!device){
        debug("No device\n");
        f->ebx = ((uint32_t)-2);
        return;
    }

    Ext2FileSystem* fs = ext2_open(BLOCK_DEVICE(device));

    if (!fs){
        debug("No fs\n");
        f->ebx = ((uint32_t)-3);
        return;
    }

    Stream* stream = ext2_file_stream_open(fs, filepath,0);

    if (stream){
        uint32_t task_id = tasks_new(stream);
        debug("New task id:");debug_i(task_id,10);debug("\n");
        f->ebx = task_id;
    } else {
        debug("Not found\n");
        f->ebx = ((uint32_t)-4);
    }

    stream_close(stream);
    ext2_close(fs);
}
