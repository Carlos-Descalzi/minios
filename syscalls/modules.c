#include "kernel/syscall.h"
#include "kernel/syscalls.h"
#include "kernel/common.h"
#include "kernel/modules.h"
#include "kernel/task.h"
#include "lib/string.h"
#include "fs/fs.h"
#include "lib/path.h"
#include "kernel/device.h"
#include "misc/debug.h"

uint32_t syscall_modload(SyscallArg arg){
    char path[PATH_SIZE];

    char* full_path = tasks_to_kernel_address(arg.ptr_arg, PATH_SIZE);

    uint16_t device_id;

    int parse_result = path_parse(full_path, &device_id, path);

    if (parse_result){
        debug("modload - bad path\n");
        return (uint32_t) -1;
    }

    Device* device = device_find_by_id(device_id);

    if (!device){
        debug("modload - no device\n");
        // unknown device
        return (uint32_t) -2;
    } 
    FileSystem* fs = fs_get_filesystem(BLOCK_DEVICE(device));

    if (fs){
        int32_t result = modules_load(fs, path);

        fs_release_filesystem(fs);

        return ((uint32_t) 10 * result);
    } else {
        // no fs
        return (uint32_t) -3;
    }

}
