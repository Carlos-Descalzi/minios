#ifndef _TASK_H_
#define _TASK_H_
#include "lib/stdint.h"
#include "kernel/context.h"
#include "kernel/paging.h"
#include "kernel/device.h"
#include "io/streams.h"
#include "kernel/isr.h"

#define TASK_STATUS_NONE    0
#define TASK_STATUS_READY   1
#define TASK_STATUS_IDLE    2
#define TASK_STATUS_RUNNING 3
#define TASK_STATUS_IOWAIT  4

/**
 * Task structure definition
 **/
typedef struct Task {
    uint32_t            tid;   
    uint32_t            status;
    CPUState            cpu_state; 
    uint32_t            parent_tid;
    PageDirectoryEntry* page_directory;
    Stream*             streams[32];        // max open files
    CharDevice*         console;            // console associated to task.
    IORequest           io_requests[4];     // max active IO requests
} Task;

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
 * Creates a new task by loading elf binary from a stream
 **/
uint32_t    tasks_new                       (Stream* exec_stream);
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
 **/
void        tasks_loop                      (void);
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
 * Adds a IO request to current task
 **/
void        tasks_add_io_request            (uint32_t type, uint32_t stream_num, 
                                            uint8_t* buffer, uint32_t size);

#endif
