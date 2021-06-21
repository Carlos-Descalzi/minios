#define NODEBUG
#include "kernel/syscall.h"
#include "lib/stdint.h"
#include "kernel/isr.h"
#include "misc/debug.h"
#include "fs/ext2.h"
#include "kernel/task.h"
#include "fs/fs.h"
#include "board/console.h"

struct ReadData {
    uint8_t stream_num;
    uint8_t *buff;
    uint32_t size;
};

void syscall_read(InterruptFrame* f){
    struct ReadData* read_data = tasks_to_kernel_address((void*)f->ebx);
    Task* task = tasks_current_task();
    uint8_t stream_num = read_data->stream_num;
    Stream* stream = task->streams[stream_num];

    debug("SYCALL - read\n");

    if (!stream){
        debug("No stream\n");
        f->ebx = ((uint32_t)-1);
    } else if (stream->async){
        debug("SYSCALL - read async\n");
        tasks_add_io_request(
            TASK_IO_REQUEST_READ, 
            stream_num, 
            read_data->buff,
            read_data->size
        );

        debug("Task send to IO wait\n");
        asm volatile("jmp do_task_exit");
    } else {
        debug("SYSCALL - read sync\n");
        uint32_t size = read_data->size;
        uint8_t* buff = tasks_to_kernel_address(read_data->buff);
        f->ebx = (uint32_t)stream_read_bytes(stream, buff, size);
    }
}
