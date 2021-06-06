#include "kernel/syscall.h"
#include "lib/stdint.h"
#include "kernel/isr.h"
#include "misc/debug.h"

static void handle_syscall(InterruptFrame f);

void syscall_init(){
    isr_install(0x31, (Isr)handle_syscall);
}

static void handle_syscall(InterruptFrame f){
    debug("Syscall called!\n");
    debug("cr3: ");debug_i(f.cr3,16);debug("\n");
    debug("cs: ");debug_i(f.cs,16);debug("\n");
    debug("ss: ");debug_i(f.source_ss,16);debug("\n");
    debug("esp: ");debug_i(f.source_esp,16);debug("\n");
    debug("flags: ");debug_i(f.flags.dwflags,16);debug("\n");
    debug("eip: ");debug_i(f.eip,16);debug("\n");
    //debug("Call number:");debug_i(call_num,16);debug("\n");
}
