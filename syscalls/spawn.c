//#define NODEBUG
#include "kernel/syscalls.h"
#include "kernel/common.h"
#include "kernel/task.h"
#include "lib/path.h"
#include "kernel/device.h"
#include "fs/fs.h"
#include "misc/debug.h"
#include "lib/string.h"
#include "lib/heap.h"

static TaskParams* copy_str_array(int len, char** str_array){

    if (len > 0){
        // these addresses are relative to the begining of the array, make them absolute
        str_array = tasks_to_kernel_address(str_array, 2048);
        for (int i=0;i<len;i++){
            str_array[i] += ((uint32_t)str_array);
        }
        TaskParams* result = task_params_from_char_array(len, str_array);
        return result;
    } 
    return NULL;
}

typedef struct {
    char* path;
    uint8_t nargs;
    char** argv;
    uint8_t nenvs;
    char** env;
} SpawnData;

uint32_t syscall_spawn(SyscallArg arg){
    uint16_t device_id;
    char filepath[PATH_SIZE];

    SpawnData* spawn_params = tasks_to_kernel_address(arg.ptr_arg, sizeof(SpawnData));

    // need to save to other variables
    // since spawn_params won't be valid after next time
    // tasks_to_kernel_address() is used
    char* path = spawn_params->path;
    uint8_t nargs = spawn_params->nargs;
    char** argv = spawn_params->argv;
    uint8_t nenvs = spawn_params->nenvs;
    char** envs = spawn_params->env;

    path = tasks_to_kernel_address(path, PATH_SIZE);

    debug("Spawning new task:");debug(path);debug("\n");

    int parse_result = path_parse(path, &device_id, filepath);

    if (parse_result){
        debug("spawn - Bad path\n");
        return (uint32_t) -1;
    }

    Device* device = device_find_by_id(device_id);

    if (!device){
        debug("spawn - No device\n");
        return (uint32_t) -2;
    }

    FileSystem* fs = fs_get_filesystem(BLOCK_DEVICE(device));

    if (!fs){
        debug("spawn - No fs\n");
        return (uint32_t) -3;
    }

    Task* current_task = tasks_current_task();

    TaskParams* new_args = copy_str_array(nargs, argv);
    TaskParams* new_env = copy_str_array(nenvs, envs);

    if (!new_env){
        new_env = task_params_copy(current_task->env);
    }

    uint32_t result;

    Stream* stream = fs_open_stream_path(fs, filepath, O_RDONLY);

    if (stream){
        result = tasks_new(current_task->tid, stream, new_args, new_env);
    } else {
        debug("spawn - Not found\n");
        result = (uint32_t) -4;
    }

    stream_close(stream);
    fs_release_filesystem(fs);

    return result;
}
