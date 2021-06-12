#define NODEBUG
#include "kernel/syscalls.h"
#include "lib/stdint.h"
#include "kernel/isr.h"
#include "misc/debug.h"
#include "kernel/task.h"

void syscall_exit(InterruptFrame* f){
    uint32_t exit_code = f->ebx;
    debug("SYSCALL - Handling task exit, code:");debug_i(f->ebx,10);debug("\n");
    tasks_finish(tasks_current_tid(), exit_code);
    debug("SYSCALL - Going to TASK EXIT\n");
    asm volatile("jmp do_task_exit");
    debug("Should not be here\n");
}
