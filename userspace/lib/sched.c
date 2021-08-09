#include "sched.h"
#include "syscall.h"
#include "stddef.h"
#include "stdlib.h"

int sched_yield(void){
    return syscall(SYS_YIELD, NULL);
}

int sched_wait_conditions(int n_conditions, WaitCondition* conditions){
    struct {
        int n_conditions;
        WaitCondition* conditions;
    } cond_data = {
        .n_conditions = n_conditions,
        .conditions = conditions
    };

    return syscall(SYS_WAITCND, &cond_data);
}

int getpid(void){
    return syscall(SYS_GETPID, NULL);
}

