#include "kernel/syscall.h"
#include "lib/stdint.h"
#include "kernel/isr.h"

static void handle_syscall(InterruptFrame f);

void syscall_init(){
    isr_install(0x5f, handle_syscall);
}

static void handle_syscall(InterruptFrame f){
}
