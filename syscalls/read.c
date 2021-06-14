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
    Task* task = tasks_current_task();
    struct ReadData* read_data = tasks_to_kernel_address((void*)f->ebx);

    uint8_t stream_num = read_data->stream_num;
    uint32_t size = read_data->size;
    uint8_t* buff = tasks_to_kernel_address(read_data->buff);

    Stream* stream = task->streams[stream_num];

    if (stream){
        f->ebx = (uint32_t)stream_read_bytes(stream, buff, size);
    } else {
        f->ebx = ((uint32_t)-1);
    }
}
