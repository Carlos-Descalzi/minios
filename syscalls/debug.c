#include "kernel/syscalls.h"
#include "misc/debug.h"
#include "kernel/task.h"
#include "kernel/common.h"

uint32_t syscall_debug (SyscallArg arg){

    char* message = arg.ptr_arg;

    message = tasks_to_kernel_address(message, PAGE_SIZE);

    debug(message);

    return 0;
}
