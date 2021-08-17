#include "kernel/syscalls.h"
#include "kernel/device.h"
#include "lib/stdlib.h"
#include "lib/string.h"
#include "misc/debug.h"
#include "kernel/task.h"

/**
 * List available devices
 **/
uint32_t syscall_devs(SyscallArg arg){
    /*
    char buff[10];
    uint8_t kind;
    uint8_t instance;

    struct {
        int index;
        char strbuffer[];
    }* devs_data = tasks_to_kernel_address((void*)f->ebx);

    int index = devs_data->index;
    char* strbuffer = devs_data->strbuffer;

    int device_count = device_count_devices();
    
    if (index > device_count-1){
        f->ebx = ((uint32_t)-1);
        return;
    }

    if (device_info(index, &kind, &instance)){
        f->ebx = ((uint32_t)-2);
        return;
    }

    strcpy(strbuffer, DEVICE_KIND_NAMES[kind]);
    strcat(strbuffer, itoa(instance,buff,10));

    if (index == device_count-1){
        f->ebx = 1;
    } else {
        f->ebx = 0;
    }
    TODO: Remove this syscall, replace by sysfs

    */
}
