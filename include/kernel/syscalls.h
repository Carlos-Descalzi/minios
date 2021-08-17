#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

#include "kernel/isr.h"

typedef union {
    uint32_t int_arg;
    void* ptr_arg;
} SyscallArg;

uint32_t syscall_read           (SyscallArg);
uint32_t syscall_write          (SyscallArg);
uint32_t syscall_open           (SyscallArg);
uint32_t syscall_close          (SyscallArg);
uint32_t syscall_exec           (SyscallArg);
uint32_t syscall_yield          (SyscallArg);
uint32_t syscall_exit           (SyscallArg);
uint32_t syscall_stat           (SyscallArg);
uint32_t syscall_getdents       (SyscallArg);
uint32_t syscall_modload        (SyscallArg);
uint32_t syscall_spawn          (SyscallArg);
uint32_t syscall_waitpid        (SyscallArg);
uint32_t syscall_devs           (SyscallArg);
uint32_t syscall_getpid         (SyscallArg);
uint32_t syscall_msg_send_sync  (SyscallArg);
uint32_t syscall_msg_recv_sync  (SyscallArg);
uint32_t syscall_msg_answer     (SyscallArg);
uint32_t syscall_msg_recv_wait  (SyscallArg);
uint32_t syscall_msg_answer     (SyscallArg);
uint32_t syscall_ioctl          (SyscallArg);
uint32_t syscall_mmap           (SyscallArg);
uint32_t syscall_debug          (SyscallArg);
uint32_t syscall_kill           (SyscallArg);
uint32_t syscall_pipe           (SyscallArg);
uint32_t syscall_waitcnd        (SyscallArg);

#endif
