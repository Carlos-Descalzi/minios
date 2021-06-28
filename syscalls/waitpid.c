#include "kernel/syscalls.h"
#include "kernel/task.h"

void syscall_waitpid(InterruptFrame* f){

    uint32_t tid = f->ebx;

    if (tid > 0){
        tasks_wait_tid(tid);
        f->ebx = 0;
        asm volatile("jmp do_task_exit");
    } else {
        f->ebx = (uint32_t)-1;
    }
}
