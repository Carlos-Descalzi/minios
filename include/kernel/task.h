#ifndef _TASK_H_
#define _TASK_H_
#include "lib/stdint.h"
#include "kernel/context.h"
#include "kernel/paging.h"
#include "io/streams.h"

#define TASK_STATUS_NONE    0
#define TASK_STATUS_READY   1
#define TASK_STATUS_IDLE    2
#define TASK_STATUS_RUNNING 3

typedef struct StreamSet {
    Stream* streams[10];
    struct StreamSet* next;
} StreamSet;

typedef struct Task {
    uint32_t tid;   // redundant
    uint32_t status;
    CPUState cpu_state; 
    //TaskStateSegment task_state;
    PageDirectoryEntry* page_directory;
    StreamSet* stream_set;
} Task;

void        tasks_init              (void);
uint32_t    tasks_current_tid       (void);
uint32_t    tasks_new               (Stream* exec_stream);
void        tasks_switch_to_task    (uint32_t task_id);

#endif
