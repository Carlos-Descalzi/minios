//#define NODEBUG
#include "kernel/syscalls.h"
#include "lib/stdint.h"
#include "kernel/isr.h"
#include "misc/debug.h"
#include "kernel/task.h"

uint32_t syscall_exit(SyscallArg arg){
    uint32_t exit_code = arg.int_arg;
    debug("exit process\n");
    tasks_finish(tasks_current_tid(), exit_code);
    asm volatile("jmp do_task_exit");
    return 0;
}
