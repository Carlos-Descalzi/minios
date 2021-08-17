#include "kernel/syscalls.h"
#include "misc/debug.h"
#include "kernel/task.h"

uint32_t syscall_yield(SyscallArg arg){
    //asm volatile("pause");
    asm volatile("jmp do_task_exit");
    return 0;
}
