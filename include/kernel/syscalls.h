#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

#include "kernel/isr.h"

void syscall_read(InterruptFrame* f);
void syscall_write(InterruptFrame* f);
void syscall_open(InterruptFrame* f);
void syscall_close(InterruptFrame* f);
void syscall_exec(InterruptFrame* f);
void syscall_exit(InterruptFrame* f);

#endif
