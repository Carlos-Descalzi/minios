#define NODEBUG
#include "kernel/syscall.h"
#include "kernel/syscalls.h"
#include "lib/stdint.h"
#include "kernel/isr.h"
#include "misc/debug.h"
#include "fs/ext2.h"
#include "kernel/task.h"
#include "fs/fs.h"
#include "board/console.h"

typedef struct {
    uint8_t stream_num;
    uint8_t *buff;
    uint32_t size;
} ReadData;

uint32_t syscall_read(SyscallArg arg){
    ReadData* read_data = tasks_to_kernel_address(arg.ptr_arg, sizeof(ReadData));
    Task* task = tasks_current_task();
    
    uint8_t stream_num = read_data->stream_num;
    uint8_t* buffer = read_data->buff;
    uint32_t size = read_data->size;

    Stream* stream = task->streams[stream_num];

    debug("SYSCALL - read\n");

    if (!stream){
        
        debug("No stream\n");
        
        return (uint32_t) -1;

    } else if (stream->async){
        debug("SYSCALL - read async\n");

        tasks_add_io_request(
            TASK_IO_REQUEST_READ | (stream->nonblocking ? TASK_IO_NOBLOCK : 0), 
            stream_num, 
            buffer,
            size
        );

        debug("Task send to IO wait\n");
        asm volatile("jmp do_task_exit");
    } else {
        debug("SYSCALL - read sync: ");debug_i(size,10);debug("\n");

        debug("task address:");debug_i(buffer,16);debug("\n");
        buffer = tasks_to_kernel_address(buffer, size);
        debug(", physical address:");debug_i(buffer,16);debug("\n");

        return (uint32_t) stream_read_bytes(stream, buffer, size);
    }
    return 0;
}
