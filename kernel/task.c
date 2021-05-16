#include "kernel/task.h"
#include "kernel/isr.h"
#include "lib/string.h"
#include "lib/heap.h"
#include "misc/debug.h"
#include "board/pic.h"

#define TASKS_MAX           32

#define TID_KERNEL          (TASKS_MAX-1)
#define TID_IDLE            1

#define TASK_STATUS_NONE    0
#define TASK_STATUS_READY   1
#define TASK_STATUS_IDLE    2
#define TASK_STATUS_RUNNING 3


typedef struct Task {
    uint32_t tid;   // redundant
    uint32_t status;

    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;

    uint32_t eip;

    uint32_t esp;
    uint32_t ebp;
    uint32_t edi;
    uint32_t esi;
    uint32_t flags;
    struct Task* next;
} Task;

/**
 * The start address for the idle loop
 **/
extern uint32_t idle_loop;
extern uint32_t task_switch;
extern uint32_t dummy;
extern void fill_task(Task* task);

static Task TASKS[TASKS_MAX];

Task* current_task; 

void tasks_init(){
    memset(TASKS,0,sizeof(TASKS));
    TASKS[TID_KERNEL].tid = TID_KERNEL;
    TASKS[TID_KERNEL].status = TASK_STATUS_RUNNING;

    fill_task(&(TASKS[TID_IDLE]));

    TASKS[TID_IDLE].tid = TID_IDLE;
    TASKS[TID_IDLE].status = TASK_STATUS_IDLE;
    TASKS[TID_IDLE].eip = (uint32_t) &idle_loop;
    // Just allocate 1kb of stack for idle task on heap
    TASKS[TID_IDLE].esp = (uint32_t)heap_alloc(1024);

    TASKS[TID_KERNEL].next = &(TASKS[TID_IDLE]);
    TASKS[TID_IDLE].next = &(TASKS[TID_KERNEL]);

    current_task = &(TASKS[TID_KERNEL]);

    isr_install(8, (Isr)&task_switch);
}

uint32_t tasks_current_tid(){
    return current_task->tid;
}

void next_task(){
    current_task->status = TASK_STATUS_IDLE;
    current_task = current_task->next;
    current_task->status = TASK_STATUS_RUNNING;
    debug("Next task:");
    debug_i(current_task->tid,10);
    debug(" ");
    debug_i(current_task->eip,16);
    debug("\n");
}

