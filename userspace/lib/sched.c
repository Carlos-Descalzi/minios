#include "sched.h"
#include "syscall.h"
#include "stddef.h"
#include "stdlib.h"

int sched_yield(void){
    return syscall(SYS_YIELD, NULL);
}

int getpid(void){
    return syscall(SYS_GETPID, NULL);
}

