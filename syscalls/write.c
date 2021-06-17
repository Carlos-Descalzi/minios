//#define NODEBUG
#include "kernel/syscall.h"
#include "lib/stdint.h"
#include "kernel/isr.h"
#include "misc/debug.h"
#include "fs/ext2.h"
#include "kernel/task.h"
#include "fs/fs.h"
#include "board/console.h"

struct WriteData {
    uint8_t stream_num;
    uint8_t *buff;
    uint32_t size;
};

void syscall_write(InterruptFrame* f){
    Task* task = tasks_current_task();
    struct WriteData* write_data = tasks_to_kernel_address((void*)f->ebx);

    uint8_t stream_num = write_data->stream_num;
    uint32_t size = write_data->size;
    uint8_t* buff = tasks_to_kernel_address(write_data->buff);

    Stream* stream = task->streams[stream_num];

    debug("Stream:");debug_i(stream_num,16);debug("\n");
    debug("Stream:");debug_i(stream,16);debug("\n");

    if (stream){
        f->ebx = (uint32_t)stream_write_bytes(stream, buff, size);
    } else {
        f->ebx = ((uint32_t)-1);
    }
}
