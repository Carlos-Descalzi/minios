#include "kernel/syscalls.h"
#include "kernel/task.h"
#include "kernel/device.h"
#include "io/streamimpl.h"
#include "kernel/paging.h"
#include "misc/debug.h"

typedef struct {
    uint32_t length;
    uint32_t prot;
    uint32_t flags;
    uint32_t fd;
    uint32_t offset;
} MmapOp;

void syscall_mmap(InterruptFrame* f){
    // TODO: Incomplete

    MmapOp* map = tasks_to_kernel_address((void*) f->ebx);

    uint32_t length = map->length;
    uint32_t fd = map->fd;

    Task* task = tasks_current_task();

    Stream* stream = task->streams[fd];

    if (!stream){
        f->ebx = ((uint32_t)-1);
        return;
    }

    if (stream->type != STREAM_TYPE_BLOCKDEV
        && stream->type != STREAM_TYPE_CHARDEV){

        // TODO: allow other fd types

        f->ebx = ((uint32_t)-2);
        return;
    }
    Device* device = DEVICE_STREAM(stream)->device;

    if (!device->mmapped){
        f->ebx = ((uint32_t)-3);
        return;
    }

    uint32_t address = device_base_address(device);

    debug("device address:");debug_i(address,16);debug("\n");

    uint32_t virtual_address = paging_map_to_task(task->page_directory, address, length, 1);

    if (!virtual_address){
        debug("Unable to map device\n");
        f->ebx = ((uint32_t)-4);
    }
    f->ebx = virtual_address;

}
