#define NODEBUG
#include "kernel/syscall.h"
#include "lib/stdint.h"
#include "kernel/isr.h"
#include "misc/debug.h"
#include "fs/ext2.h"
#include "kernel/task.h"
#include "fs/fs.h"
#include "board/console.h"

void syscall_read(InterruptFrame* f){
    Stream *stream;
    Task* task = tasks_current_task();
    struct {
        uint8_t stream_num;
        uint8_t *buff;
        uint32_t size;
    } * read_data = ((void*)f->ebx);

    stream = task->streams[read_data->stream_num];

    f->ebx = (uint32_t)stream_read_bytes(stream, read_data->buff, read_data->size);
}
