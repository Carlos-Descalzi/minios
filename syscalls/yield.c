#include "kernel/syscalls.h"
#include "misc/debug.h"
#include "kernel/task.h"

void syscall_yield(InterruptFrame* frame){
    debug("Calling yield\n");
    asm volatile("jmp do_task_exit");
}
