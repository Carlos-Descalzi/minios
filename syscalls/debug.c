#include "kernel/syscalls.h"
#include "misc/debug.h"
#include "kernel/task.h"
#include "kernel/common.h"

void syscall_debug (InterruptFrame* f){

    char* message = (char*) f->ebx;

    message = tasks_to_kernel_address(message, PAGE_SIZE);

    debug(message);

}
