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

typedef uint32_t (*SysCall)(SyscallArg);

static SysCall syscalls[0x100];
static void handle_syscall(InterruptFrame* f, void* data);

void syscall_init(){
    memset(syscalls,0,sizeof(syscalls));
    
    syscalls[0x00] = syscall_read;
    syscalls[0x01] = syscall_write;
    syscalls[0x02] = syscall_open;
    syscalls[0x03] = syscall_close;
    syscalls[0x04] = syscall_exec;
    syscalls[0x05] = syscall_yield;
    syscalls[0x06] = syscall_getpid;
    syscalls[0x07] = syscall_waitcnd;
    syscalls[0x08] = syscall_lseek;

    syscalls[0x09] = syscall_mmap;
    syscalls[0x10] = syscall_ioctl;
    // ....
    syscalls[0x16] = syscall_pipe;
    // ....
    syscalls[0x20] = syscall_debug;
    syscalls[0x23] = syscall_sleep;
    // ....
    syscalls[0x3e] = syscall_kill;
    //
    syscalls[0x40] = syscall_gettime;
    // ....
    syscalls[0x4d] = syscall_stat;
    syscalls[0x4e] = syscall_getdents;
    // ....
    syscalls[0x60] = syscall_msg_send_sync;
    syscalls[0x61] = syscall_msg_recv_sync;
    syscalls[0x62] = syscall_msg_recv_wait;
    syscalls[0x63] = syscall_msg_answer;
    // ....
    syscalls[0x70] = syscall_devs;
    syscalls[0x80] = syscall_spawn;
    syscalls[0x81] = syscall_waitpid;
    // ... to be filled
    // ... leave these for last
    syscalls[0x90] = syscall_modload;
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
        SyscallArg arg = { .int_arg = f->ebx };
        f->ebx = syscalls[f->eax](arg);
    } else {
        debug("\tUnknown system call:");debug_i(f->eax,16);debug("\n");
    }
}

