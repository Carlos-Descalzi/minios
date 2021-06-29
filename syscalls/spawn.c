//#define NODEBUG
#include "kernel/syscalls.h"
#include "kernel/task.h"
#include "lib/path.h"
#include "kernel/device.h"
#include "fs/fs.h"
#include "misc/debug.h"
#include "lib/string.h"
#include "lib/heap.h"

static char** copy_str_array(int len, char** str_array){
    
    if (len > 0){
        char** array = tasks_to_kernel_address(str_array);
        char* last_str = ((char*)array) + ((uint32_t)array[len-1]);
        int total_size = strlen(last_str) + 1 + ((uint32_t)array[len-1]);
        void* new_block = heap_alloc(total_size);
        memcpy(new_block, array, total_size);
        return (char**)new_block;
    } 
    return NULL;
}

void syscall_spawn(InterruptFrame* f){
    uint16_t device_id;
    char filepath[256];
    struct {
        char* path;
        uint8_t nargs;
        char** argv;
        uint8_t nenvs;
        char** env;
    } *spawn_params = tasks_to_kernel_address((void*)f->ebx);

    // need to save to other variables
    // since spawn_params won't be valid after next time
    // tasks_to_kernel_address() is used
    char* path = spawn_params->path;
    uint8_t nargs = spawn_params->nargs;
    char** argv = spawn_params->argv;
    uint8_t nenvs = spawn_params->nenvs;
    char** envs = spawn_params->env;

    path = tasks_to_kernel_address(path);

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

    FileSystem* fs = fs_get_filesystem(BLOCK_DEVICE(device));

    if (!fs){
        debug("No fs\n");
        f->ebx = ((uint32_t)-3);
        return;
    }
    char** new_argv = copy_str_array(nargs, argv);
    char** new_envs = copy_str_array(nenvs, envs);

    Stream* stream = fs_file_stream_open(fs, filepath,0);

    if (stream){
        uint32_t task_id = tasks_new(stream, nargs, new_argv, nenvs, new_envs);
        debug("New task id:");debug_i(task_id,10);debug("\n");
        f->ebx = task_id;
    } else {
        debug("Not found\n");
        f->ebx = ((uint32_t)-4);
    }

    stream_close(stream);
    fs_close(fs);
}
