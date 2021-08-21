#include "kernel/syscalls.h"
#include "kernel/task.h"
#include "io/streams.h"
#include "misc/debug.h"

typedef struct {
    uint8_t stream_num;
    uint64_t offset;
    int32_t whence;
} SeekData;

uint32_t syscall_lseek (SyscallArg arg){

    SeekData* seek_data = tasks_to_kernel_address(arg.ptr_arg, sizeof(SeekData));

    Task* task = tasks_current_task();

    Stream* stream = task->streams[seek_data->stream_num];


    if (!stream){
        debug("No stream\n");
        return (uint32_t) -1;
    }

    return stream_seek(stream, seek_data->offset, seek_data->whence);
}
