#ifndef _TASK_H_
#define _TASK_H_
#include "lib/stdint.h"
#include "kernel/context.h"
#include "kernel/paging.h"

#define TASK_STATUS_NONE    0
#define TASK_STATUS_READY   1
#define TASK_STATUS_IDLE    2
#define TASK_STATUS_RUNNING 3

typedef struct Task {
    uint32_t tid;   // redundant
    uint32_t status;
    TaskStateSegment task_state;
    PageTableEntry* page_table;
} Task;

void        tasks_init();
uint32_t    tasks_current_tid();

#endif
