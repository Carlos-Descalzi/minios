#define NODEBUG
#include "kernel/syscall.h"
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
} WriteData;

void syscall_write(InterruptFrame* f){
    debug("SYSCALL - write\n");
    Task* task = tasks_current_task();
    WriteData* write_data = tasks_to_kernel_address((void*)f->ebx, sizeof(WriteData));

    uint8_t stream_num = write_data->stream_num;
    uint32_t size = write_data->size;
    uint8_t* buff = write_data->buff;

    Stream* stream = task->streams[stream_num];

    if (stream){
        buff = tasks_to_kernel_address(buff, size);

        f->ebx = (uint32_t)stream_write_bytes(stream, buff, size);
    } else {
        debug("No stream\n");
        f->ebx = ((uint32_t)-1);
    }
}
