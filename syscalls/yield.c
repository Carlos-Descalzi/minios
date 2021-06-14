#include "kernel/syscalls.h"
#include "misc/debug.h"
#include "kernel/task.h"

void syscall_yield(InterruptFrame* frame){
    debug("Calling yield\n");
    tasks_next();
    frame->eax = 0;
}
