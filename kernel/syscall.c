#define NODEBUG
#include "kernel/syscall.h"
#include "lib/stdint.h"
#include "kernel/isr.h"
#include "misc/debug.h"
#include "fs/ext2.h"
#include "kernel/task.h"
#include "fs/fs.h"
#include "board/console.h"
#include "kernel/syscalls.h"
#include "lib/string.h"

typedef void (*SysCall)(InterruptFrame *f);

static SysCall syscalls[0x100];
static void handle_syscall(InterruptFrame* f, void* data);
static void syscall_debug(InterruptFrame* f);

void syscall_init(){
    memset(syscalls,0,sizeof(syscalls));
    
    syscalls[0] = syscall_read;
    syscalls[1] = syscall_write;
    syscalls[2] = syscall_open;
    syscalls[3] = syscall_close;
    syscalls[4] = syscall_exec;
    syscalls[5] = syscall_yield;
    // ....
    syscalls[77] = syscall_stat;
    syscalls[78] = syscall_getdents;
    // ....
    syscalls[0x80] = syscall_spawn;
    syscalls[0x81] = syscall_waitpid;
    // ... to be filled
    // ... leave these for last
    syscalls[0x90] = syscall_modload;
    syscalls[0x98] = syscall_debug;
    syscalls[0x99] = syscall_exit;

    isr_install(0x31, handle_syscall, NULL);
}

static void handle_syscall(InterruptFrame* f, void* data){
    debug("SYSCALL - Syscall called!\n");
    debug("\teax: ");debug_i(f->eax,16);debug("\n");
    debug("\tebx: ");debug_i(f->ebx,16);debug("\n");
    debug("\tecx: ");debug_i(f->ecx,16);debug("\n");
    debug("\tcr3: ");debug_i(f->cr3,16);debug("\n");
    debug("\tcs: ");debug_i(f->cs,16);debug("\n");
    debug("\tss: ");debug_i(f->source_ss,16);debug("\n");
    debug("\tesp: ");debug_i(f->source_esp,16);debug("\n");
    debug("\tflags: ");debug_i(f->flags.dwflags,16);debug("\n");
    debug("\teip: ");debug_i(f->eip,16);debug("\n");

    if (syscalls[f->eax]){
        syscalls[f->eax](f);
    } else {
        debug("\tUnknown system call:");debug_i(f->eax,16);debug("\n");
    }
}


static void syscall_debug(InterruptFrame* f){
    uint8_t* str_ptr = tasks_to_kernel_address((uint8_t*)f->ebx);
    console_print(str_ptr);
    f->ebx = 0;
    f->eax = 0;
}
