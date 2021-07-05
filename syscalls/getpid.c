#include "kernel/syscalls.h"
#include "kernel/task.h"

void syscall_getpid(InterruptFrame* f){
    f->ebx = tasks_current_tid();
}
