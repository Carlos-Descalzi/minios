#define NODEBUG
#include "kernel/syscall.h"
#include "lib/stdint.h"
#include "kernel/isr.h"
#include "misc/debug.h"
#include "fs/ext2.h"
#include "kernel/task.h"
#include "fs/fs.h"
#include "board/console.h"
#include "lib/string.h"

#define RESOURCE_TYPE_RAW   0x00
#define RESOURCE_TYPE_FS    0x01

static int next_stream_pos(Task *task){
    int i;
    for (i=0;i<32;i++){
        if (!task->streams[i]){
            return i;
        }
    }
    return -1;
}

void syscall_open(InterruptFrame* f){
    char *path;
    Device* device;
    int pos;
    Task* task = tasks_current_task();
    struct {
        uint8_t resource_type;
        uint8_t device_kind:7,
            device_resource:1;
        uint8_t device_instance;
        uint8_t mode;
        const char* path;
    } * open_data = tasks_to_kernel_address((void*)f->ebx);

    pos = next_stream_pos(task);

    if (pos < 0){
        f->ebx = ((uint32_t)-1);
        return;
    }

    device = device_find(open_data->device_kind, open_data->device_instance);

    if (!device){
        f->ebx = ((uint32_t)-2);
        return;
    }

    if (open_data->resource_type == RESOURCE_TYPE_RAW){
        f->ebx = ((uint32_t)-3);
        // TODO: not implemented yet
        return;
    }

    if (!open_data->path){
        f->ebx = ((uint32_t)-4);
        return;
    }

    char* full_path = tasks_to_kernel_address((void*)open_data->path);

    char* file_path = strchr(full_path,':')+1;

    debug("Opening file ");debug(file_path);debug("\n");

    Ext2FileSystem* fs = fs_get_filesystem(device);
    task->streams[pos] = ext2_file_stream_open(fs, file_path, open_data->mode);
    
    if (!task->streams[pos]){
        debug("File not found\n");
        f->ebx = ((uint32_t)-5);
        return;
    }
    debug("File opened:");debug_i(pos,10);debug("\n");

    f->ebx = pos;
}
