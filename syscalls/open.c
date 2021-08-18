//#define NODEBUG
#include "kernel/syscall.h"
#include "kernel/syscalls.h"
#include "kernel/common.h"
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

uint32_t syscall_open(SyscallArg arg){
    char path[PATH_SIZE];
    uint16_t device_id;

    debug("Syscall open");

    OpenData* open_data = tasks_to_kernel_address(arg.ptr_arg, sizeof(OpenData));

    uint32_t flags = open_data->flags;
    char* full_path = open_data->path;

    Task* task = tasks_current_task();
    int pos = next_stream_pos(task);

    if (pos < 0){
        return (uint32_t) -1;
    }

    full_path = tasks_to_kernel_address(full_path, PATH_SIZE);
    debug("\tpath:");debug(full_path);debug(", flags:");debug_i(flags,16);debug("\n");
    int parse_result = path_parse(full_path, &device_id, path);

    if (parse_result){
        return (uint32_t) -2;
    }

    Device* device = device_find_by_id(device_id);

    if (!device){
        return (uint32_t) -3;
    }

    Stream* stream;

    if (!strlen(path)){
        debug("Opening raw device\n");

        stream = device_stream_open(device, open_data->flags);

        if (!stream){
            return (uint32_t) -4;
        }

    } else {
        debug("Opening file ");debug(path);debug("\n");

        FileSystem* fs = fs_get_filesystem(BLOCK_DEVICE(device));

        if (!fs){
            debug("File system not found\n");
            return (uint32_t) -5;
        }

        stream = fs_open_stream_path(fs, path, flags);
        
        if (!stream){
            debug("File not found\n");
            return (uint32_t) -6;
        }
        debug("File opened:");debug_i(pos,10);debug("\n");
    }
    debug("Stream open with fd: ");debug_i(pos,10);debug("\n");

    task->streams[pos] = stream;

    return pos;
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
