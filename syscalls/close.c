#define NODEBUG
#include "kernel/syscall.h"
#include "lib/stdint.h"
#include "kernel/isr.h"
#include "misc/debug.h"
#include "fs/ext2.h"
#include "kernel/task.h"
#include "fs/fs.h"
#include "board/console.h"

void syscall_close(InterruptFrame* f){
    Task* task = tasks_current_task();
    uint32_t stream_num = f->ebx;

    if (task->streams[stream_num]){
        stream_close(task->streams[stream_num]);
        task->streams[stream_num] = NULL;
        f->ebx = 0;
    } else {
        f->ebx = ((uint32_t)-1);
    }
}
