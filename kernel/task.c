#include "kernel/task.h"
#include "kernel/isr.h"
#include "lib/string.h"
#include "lib/heap.h"
#include "misc/debug.h"
#include "board/pic.h"

#define TASKS_MAX           32
#define TID_KERNEL          0

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
    current_task = NULL;
}

uint32_t tasks_current_tid(){
    return current_task ? current_task->tid : 0;
}

void next_task(){
    current_task->status = TASK_STATUS_IDLE;
    current_task = current_task->next;
    current_task->status = TASK_STATUS_RUNNING;
    debug("Next task:");
    debug_i(current_task->tid,10);
    debug(" ");
    debug_i(current_task->cpu_state.eip,16);
    debug("\n");
}

