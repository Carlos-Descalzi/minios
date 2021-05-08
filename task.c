#include "task.h"
#include "string.h"
#include "stdint.h"
#include "isr.h"
#include "debug.h"
#include "pic.h"
#include "stdlib.h"

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
    //char buff[32];
    memset(TASKS,0,sizeof(TASKS));
    TASKS[TID_KERNEL].tid = TID_KERNEL;
    TASKS[TID_KERNEL].status = TASK_STATUS_RUNNING;

    fill_task(&(TASKS[TID_IDLE]));

    TASKS[TID_IDLE].tid = TID_IDLE;
    TASKS[TID_IDLE].status = TASK_STATUS_IDLE;
    TASKS[TID_IDLE].eip = (uint32_t) &idle_loop;
    TASKS[TID_IDLE].esp = 0x30100;  // TODO fix when have paging.

    TASKS[TID_KERNEL].next = &(TASKS[TID_IDLE]);
    TASKS[TID_IDLE].next = &(TASKS[TID_KERNEL]);

    /*debug("Idle task:");
    debug(itoa(TASKS[TID_IDLE].eip,buff,16));
    debug("\n");*/

    current_task = &(TASKS[TID_KERNEL]);

    isr_install(8, (Isr)&task_switch);
}

uint32_t tasks_current_tid(){
    return current_task->tid;
}

void next_task(){
    char buff[32];
    current_task->status = TASK_STATUS_IDLE;
    /*
    debug("Current task:");
    debug(itoa(current_task->tid,buff,10));
    debug(" ");
    debug(itoa(current_task->eip,buff,16));
    debug("\n");
    */
    current_task = current_task->next;
    current_task->status = TASK_STATUS_RUNNING;
    debug("Next task:");
    debug(itoa(current_task->tid,buff,10));
    debug(" ");
    debug(itoa(current_task->eip,buff,16));
    debug("\n");
    //debug("switch \n");
}

void dump_eflags(uint32_t eflags){
    char buff[32];
    debug(itoa(eflags,buff,16));
    debug("\n");
}
