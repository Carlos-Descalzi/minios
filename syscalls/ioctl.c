#include "kernel/syscalls.h"
#include "kernel/task.h"
#include "kernel/device.h"
#include "io/streamimpl.h"

typedef struct {
    uint32_t fd;
    uint32_t request;
    void* data;
} IoctlData;

uint32_t syscall_ioctl(SyscallArg arg){

    IoctlData* op = tasks_to_kernel_address(arg.ptr_arg, sizeof(IoctlData));

    uint32_t fd = op->fd;
    uint32_t request = op->request;
    void* data = op->data;

    Task* task = tasks_current_task();

    Stream* stream = task->streams[fd];

    if (!stream){
        return (uint32_t) -1;
    }

    if (stream->type != STREAM_TYPE_CHARDEV
        && stream->type != STREAM_TYPE_BLOCKDEV){

        // TODO: allow other fd types
        
        return (uint32_t) -2;
    }

    Device* device = DEVICE_STREAM(stream)->device;

    data = tasks_to_kernel_address(data, PAGE_SIZE);

    return device_setopt(device, request, data);
}
