#include "sched.h"
#include "syscall.h"
#include "stddef.h"

int sched_yield(void){
    return syscall(SYS_YIELD, NULL);
}
