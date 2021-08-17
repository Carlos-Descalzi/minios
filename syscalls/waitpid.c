#include "kernel/syscalls.h"
#include "kernel/task.h"

uint32_t syscall_waitpid(SyscallArg arg){

    uint32_t tid = arg.ptr_arg;

    if (tid > 0){
        tasks_wait_tid(tid);
        asm volatile("jmp do_task_exit");
    } else {
        return (uint32_t) -1;
    }
    return 0;
}
