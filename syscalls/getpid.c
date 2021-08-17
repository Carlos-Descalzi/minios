#include "kernel/syscalls.h"
#include "kernel/task.h"

uint32_t syscall_getpid(SyscallArg arg){
    return tasks_current_tid();
}
