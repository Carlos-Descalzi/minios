#ifndef _TASK_H_
#define _TASK_H_
#include "lib/stdint.h"
#include "kernel/context.h"
#include "kernel/paging.h"
#include "kernel/device.h"
#include "io/streams.h"
#include "kernel/isr.h"
#include "lib/params.h"
#include "lib/list.h"

#define TASK_STATUS_NONE    0
#define TASK_STATUS_READY   1
#define TASK_STATUS_IDLE    2
#define TASK_STATUS_RUNNING 3
#define TASK_STATUS_IOWAIT  4
#define TASK_STATUS_WAITCND 5

typedef struct {
    uint32_t            source;
    uint32_t            target;
    uint32_t            number:31,
                        has_more:1;
    char                body[1024 - sizeof(uint32_t)*3];
} Message;

typedef union {
    void*       ptr_data;
    uint32_t    int_data;
} ConditionData;

#define COND_TYPE_FD    1
#define COND_TYPE_MSG   2

typedef struct {
    int cond_type;
    int fd;
} WaitConditionItem;

typedef struct {
    int n_conditions;
    WaitConditionItem items[];
} WaitCondition;

typedef struct Task Task;

typedef int (*Condition)(Task*);

#define MESSAGE_SIZE    (sizeof(Message)+1024)
/**
 * Task structure definition
 **/
struct Task {
    uint32_t            tid;   
    uint32_t            status;
    CPUState            cpu_state; 
    uint32_t            parent_tid;
    PageDirectoryEntry* page_directory;
    TaskParams*         args;
    TaskParams*         env;
    Stream*             streams[32];        // max open files
    CharDevice*         console;            // console associated to task.
    IORequest           io_requests[4];     // max active IO requests
    Condition           waitcond;           // wait condition
    ConditionData       cond_data;
    ListNode*           incoming_messages;
};

typedef int (*TaskVisitor)                  (Task*, void*);

/**
 * Initializes task scheduler
 **/
void        tasks_init                      (void);
/**
 * Returns the current task id
 **/
uint32_t    tasks_current_tid               (void);
/**
 * Returns the current task
 **/
Task*       tasks_current_task              (void);
/**
 * Returns a task by id
 **/
Task*       tasks_get_task_by_tid           (uint32_t task_id);
/**
 * Creates a new task by loading elf binary from a stream
 **/
uint32_t    tasks_new                       (Stream* exec_stream,
                                            TaskParams* args,
                                            TaskParams* env);
/**
 * Finishes a given task
 **/
void        tasks_finish                    (uint32_t task_id, uint32_t exit_code);
/**
 * Updates current task with information taken from
 * an Interrupt frame
 **/
void        tasks_update_current            (InterruptFrame* frame);
/**
 * Scheduler main loop function, it runs one cycle of task looping.
 * Returns 1 when it ran any task, otherwise 0.
 **/
int         tasks_loop                      (void);
/**
 * Converts an address from current tasks's address space
 * to kernel space by mapping it to a well known kernel page
 **/
void*       tasks_to_kernel_address         (void* address);
/**
 * Converts an address from task's address space
 * to kernel space by mapping it to a well known kernel page
 **/
void*       tasks_task_to_kernel_adddress   (uint32_t tid, void* address);
/**
 * Adds a IO request to current task and puts task in wait state.
 **/
void        tasks_add_io_request            (uint32_t type, uint32_t stream_num, 
                                            uint8_t* buffer, uint32_t size);
/**
 * Puts task in wait state until a given task identified by its id
 * finishes.
 **/
void        tasks_wait_tid                  (uint32_t tid);
/**
 * Places a message on target's inbox and puts task in wait state
 * until message is answered
 **/
void        tasks_send_message_sync         (Message* message);
/**
 * Sends a message without waiting response
 **/
void        tasks_send_message              (Message* message);
/**
 * If there is incoming message, copies it to message parameter, 
 * returning 0, otherwise returns non zero.
 **/
int         tasks_check_for_message         (Message* message);
/**
 * Puts task in wait state until message arrives
 **/
int         tasks_wait_message              (Message* message);
/**
 * Returns the amount of tasks in the system
 **/
int         tasks_count                     (void);
/**
 * Iterates over tasks
 **/
void        tasks_iter_tasks                (TaskVisitor visitor, void* data);
#endif
