#define NODEBUG
#include "kernel/syscall.h"
#include "lib/stdint.h"
#include "kernel/isr.h"
#include "misc/debug.h"
#include "fs/ext2.h"
#include "kernel/task.h"
#include "fs/fs.h"
#include "board/console.h"

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
    Device* device;
    int pos;
    Task* task = tasks_current_task();
    struct {
        uint8_t resource_type;
        uint8_t device_kind;
        uint8_t device_instance;
        uint8_t mode;
        const char path[1];
    } * open_data = ((void*)f->ebx);

    pos = next_stream_pos(task);

    if (pos >= 0){
        device = device_find(open_data->device_kind, open_data->device_instance);

        if (device){
            if (open_data->resource_type == RESOURCE_TYPE_RAW){
            } else {
                Ext2FileSystem* fs = fs_get_filesystem(device);
                task->streams[pos] = ext2_file_stream_open(fs, open_data->path, open_data->mode);
                f->ebx = pos;
            }
        } else {
            f->ebx = ((uint32_t)-2);
        }
    } else {
        f->ebx = ((uint32_t)-1);
    }
}
