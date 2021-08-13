#include "kernel/syscalls.h"
#include "kernel/task.h"
#include "kernel/device.h"
#include "io/streamimpl.h"

typedef struct {
    uint32_t fd;
    uint32_t request;
    void* data;
} IoctlData;

void syscall_ioctl(InterruptFrame* f){

    IoctlData* op = tasks_to_kernel_address((void*) f->ebx, sizeof(IoctlData));

    uint32_t fd = op->fd;
    uint32_t request = op->request;
    void* data = op->data;

    Task* task = tasks_current_task();

    Stream* stream = task->streams[fd];

    if (!stream){
        f->ebx = ((uint32_t)-1);
        return;
    }

    if (stream->type != STREAM_TYPE_CHARDEV
        && stream->type != STREAM_TYPE_BLOCKDEV){

        // TODO: allow other fd types
        
        f->ebx = ((uint32_t)-2);
        return;
    }

    Device* device = DEVICE_STREAM(stream)->device;

    data = tasks_to_kernel_address(data, 4096);

    f->ebx = device_setopt(device, request, data);

}
