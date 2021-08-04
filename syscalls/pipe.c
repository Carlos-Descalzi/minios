#include "kernel/syscalls.h"
#include "lib/stdint.h"

void syscall_pipe (InterruptFrame* f){

    f->ebx = (uint32_t) -1;
}
