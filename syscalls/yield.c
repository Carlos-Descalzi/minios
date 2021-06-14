#include "kernel/syscalls.h"
#include "misc/debug.h"

void syscall_yield(InterruptFrame* frame){
    debug("Calling yield\n");
    frame->eax = 0;
}
