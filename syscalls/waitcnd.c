#define NODEBUG
#include "kernel/syscalls.h"
#include "kernel/task.h"
#include "misc/debug.h"

typedef struct {
    int n_conditions;
    WaitConditionItem* conditions;
} WaitConditionData;

void syscall_waitcnd (InterruptFrame* f){
    debug("SYSCALL - waitcnd\n");

    WaitConditionData* cond_data = tasks_to_kernel_address( (void*) f->ebx);

    int n_conditions = cond_data->n_conditions;
    WaitConditionItem* conditions = cond_data->conditions;

    conditions = tasks_to_kernel_address(conditions);

    WaitCondition* wait_condition = heap_alloc(
        sizeof(WaitCondition) 
        + sizeof(WaitConditionItem) * n_conditions
    );

    wait_condition->n_conditions = n_conditions;

    memcpy(wait_condition->items, conditions, sizeof(WaitConditionItem) * n_conditions);

    tasks_wait_conditions(wait_condition);

    asm volatile("jmp do_task_exit");
}
