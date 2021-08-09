#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

#include "kernel/isr.h"

void syscall_read           (InterruptFrame* f);
void syscall_write          (InterruptFrame* f);
void syscall_open           (InterruptFrame* f);
void syscall_close          (InterruptFrame* f);
void syscall_exec           (InterruptFrame* f);
void syscall_yield          (InterruptFrame* f);
void syscall_exit           (InterruptFrame* f);
void syscall_stat           (InterruptFrame* f);
void syscall_getdents       (InterruptFrame* f);
void syscall_modload        (InterruptFrame* f);
void syscall_spawn          (InterruptFrame* f);
void syscall_waitpid        (InterruptFrame* f);
void syscall_devs           (InterruptFrame* f);
void syscall_getpid         (InterruptFrame* f);
void syscall_msg_send_sync  (InterruptFrame* f);
void syscall_msg_recv_sync  (InterruptFrame* f);
void syscall_msg_answer     (InterruptFrame* f);
void syscall_msg_recv_wait  (InterruptFrame* f);
void syscall_msg_answer     (InterruptFrame* f);
void syscall_ioctl          (InterruptFrame* f);
void syscall_mmap           (InterruptFrame* f);
void syscall_debug          (InterruptFrame* f);
void syscall_kill           (InterruptFrame* f);
void syscall_pipe           (InterruptFrame* f);
void syscall_waitcnd        (InterruptFrame* f);

#endif
