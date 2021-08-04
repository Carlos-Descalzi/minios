//#define NODEBUG
#include "kernel/syscall.h"
#include "lib/stdint.h"
#include "kernel/isr.h"
#include "misc/debug.h"
#include "fs/ext2.h"
#include "kernel/task.h"
#include "fs/fs.h"
#include "board/console.h"
#include "lib/string.h"
#include "lib/path.h"
#include "io/streamimpl.h"

typedef struct {
    uint32_t flags;
    char* path;
} OpenData;

static int next_stream_pos(Task *task);

void syscall_open(InterruptFrame* f){
    char path[256];

    debug("Syscall open");

    Task* task = tasks_current_task();
    OpenData* open_data = tasks_to_kernel_address((void*)f->ebx);
    uint32_t flags = open_data->flags;
    char* full_path = tasks_to_kernel_address(open_data->path);

    debug("\tpath:");debug(full_path);debug(", flags:");debug_i(flags,16);debug("\n");

    int pos = next_stream_pos(task);

    if (pos < 0){
        f->ebx = ((uint32_t)-1);
        return;
    }

    uint16_t device_id;

    if (path_parse(full_path, &device_id, path)){
        f->ebx = ((uint32_t)-1);
        return;
    }

    Device* device = device_find_by_id(device_id);

    if (!device){
        f->ebx = ((uint32_t)-2);
        return;
    }

    Stream* stream;

    if (!strlen(path)){
        debug("Opening raw device\n");

        stream = device_stream_open(device, open_data->flags);

        if (!stream){
            f->ebx = ((uint32_t)-4);
            return;
        }

    } else {
        debug("Opening file ");debug(path);debug("\n");

        FileSystem* fs = fs_get_filesystem(BLOCK_DEVICE(device));

        if (!fs){
            debug("File system not found\n");
            f->ebx = ((uint32_t)-5);
            return;
        }

        stream = fs_open_stream_path(fs, path, flags);
        
        if (!stream){
            debug("File not found\n");
            f->ebx = ((uint32_t)-5);
            return;
        }
        debug("File opened:");debug_i(pos,10);debug("\n");
    }

    task->streams[pos] = stream;
    f->ebx = pos;
}

static int next_stream_pos(Task *task){
    int i;
    for (i=0;i<32;i++){
        if (!task->streams[i]){
            return i;
        }
    }
    return -1;
}
