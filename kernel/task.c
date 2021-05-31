#include "kernel/task.h"
#include "kernel/isr.h"
#include "lib/string.h"
#include "lib/heap.h"
#include "misc/debug.h"
#include "board/pic.h"
#include "bin/elf.h"
#include "kernel/paging.h"

#define TASKS_MAX           32
#define TID_KERNEL          0

#define PAGE_CODE       0x00
#define PAGE_DATA       0x01
#define PAGE_STACK      0x02

TaskStateSegment* TSS = (TaskStateSegment*)0x500;

/**
 * The start address for the idle loop
 **/
//extern uint32_t idle_loop;
//extern uint32_t task_switch;
//extern uint32_t dummy;
//extern void fill_task(Task* task);

static Task TASKS[TASKS_MAX];

Task* current_task; 

void tasks_init(){
    memset(TASKS,0,sizeof(TASKS));
    current_task = NULL;
    TSS->ss0 = 0x10;
}

uint32_t tasks_current_tid(){
    return current_task ? current_task->tid : 0;
}

void next_task(){
    /*
    current_task->status = TASK_STATUS_IDLE;
    current_task = current_task->next;
    current_task->status = TASK_STATUS_RUNNING;
    debug("Next task:");
    debug_i(current_task->tid,10);
    debug(" ");
    debug_i(current_task->cpu_state.eip,16);
    debug("\n");
    */
}

static Task* get_next_free_task(){
    int i;
    for (i=0;i<TASKS_MAX;i++){
        if (!TASKS[i].tid){
            return &(TASKS[i]);
        }
    }
    debug("No more room for tasks\n");
    return NULL;
}
static uint32_t new_task_id(){
    uint32_t tid=0;
    int i;
    for (i=0;i<TASKS_MAX;i++){
        tid = max(tid,TASKS[i].tid);
    }
    return tid+1;
}

uint32_t tasks_new(Stream* exec_stream){
    Task* task;
    ElfHeader header;
    ElfProgramHeader prg_header;
    PageTableEntry* page_table;
    VirtualAddress v_address;
    task = get_next_free_task();

    if (!task){
        return 0;
    }
    memset(task,0,sizeof(Task));
    debug("Reading executable headers:\n");
    elf_read_header(exec_stream, &header);
    elf_read_program_header(exec_stream, &header, 0, &prg_header);

    task->tid = new_task_id();
    debug("Allocating page directory\n");
    task->page_directory = paging_new_page_directory(1);
    debug("Allocating table\n");
    paging_alloc_table(&(task->page_directory[0]));
    page_table = (PageTableEntry*)(task->page_directory[0].page_table_address << 12);

    debug("Loading code into memory\n");
    paging_load_code(exec_stream, page_table);
    debug("Setting up task\n");

    task->cpu_state.cr3 = (uint32_t)task->page_directory;
    task->cpu_state.cs = 0x23;
    task->cpu_state.ds = 0x2B;
    task->cpu_state.source_ss = 0x2B;
    v_address.page_dir_index = 0;
    v_address.page_index = 0;
    v_address.offset = 0xFFF;
    task->cpu_state.source_esp = v_address.address;
    task->cpu_state.eip = prg_header.virtual_address;

    return task->tid;
}

void tasks_switch_to_task(uint32_t task_id){
    int i;
    if (current_task){
    } else {
        for (i=0;i<TASKS_MAX;i++){
            if (TASKS[i].tid == task_id){
                current_task = &(TASKS[i]);
                break;
            }
        }
        if (current_task){
            debug("Switching to task\n");
        }
    }
}
