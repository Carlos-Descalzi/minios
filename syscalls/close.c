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

uint32_t syscall_close(SyscallArg arg){
    Task* task = tasks_current_task();
    uint32_t stream_num = arg.int_arg;

    if (task->streams[stream_num]){
        stream_close(task->streams[stream_num]);
        task->streams[stream_num] = NULL;
        return 0;
    } else {
        return (uint32_t) -1;
    }
}
