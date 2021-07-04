#define NODEBUG
#include "kernel/syscalls.h"
#include "lib/stdint.h"
#include "kernel/isr.h"
#include "misc/debug.h"
#include "kernel/task.h"

void syscall_exit(InterruptFrame* f){
    uint32_t exit_code = f->ebx;
    tasks_finish(tasks_current_tid(), exit_code);
    asm volatile("jmp do_task_exit");
}
