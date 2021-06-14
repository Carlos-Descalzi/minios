#ifndef _TASK_H_
#define _TASK_H_
#include "lib/stdint.h"
#include "kernel/context.h"
#include "kernel/paging.h"
#include "io/streams.h"
#include "kernel/isr.h"

#define TASK_STATUS_NONE    0
#define TASK_STATUS_READY   1
#define TASK_STATUS_IDLE    2
#define TASK_STATUS_RUNNING 3

typedef struct Task {
    uint32_t            tid;   
    uint32_t            status;
    CPUState            cpu_state; 
    PageDirectoryEntry* page_directory;
    Stream*             streams[32];    // max open files
} Task;

void        tasks_init              (void);
uint32_t    tasks_current_tid       (void);
Task*       tasks_current_task      (void);
uint32_t    tasks_new               (Stream* exec_stream);
void        tasks_finish            (uint32_t task_id, uint32_t exit_code);
void        tasks_update_current    (InterruptFrame* frame);
void        tasks_loop              (void);
void*       tasks_to_kernel_address (void* address);
void        tasks_next              (void);

#endif
