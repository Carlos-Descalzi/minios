#include "kernel/syscalls.h"
#include "misc/debug.h"
#include "kernel/task.h"

void syscall_yield(InterruptFrame* frame){
    asm volatile("pause");
    asm volatile("jmp do_task_exit");
}
