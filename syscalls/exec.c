#define NODEBUG
#include "kernel/syscall.h"
#include "kernel/syscalls.h"
#include "lib/stdint.h"
#include "kernel/isr.h"
#include "misc/debug.h"
#include "fs/ext2.h"
#include "kernel/task.h"
#include "fs/fs.h"
#include "board/console.h"

uint32_t syscall_exec(SyscallArg arg){
}
