//#define NODEBUG
#include "kernel/task.h"
#include "kernel/isr.h"
#include "lib/string.h"
#include "lib/heap.h"
#include "misc/debug.h"
#include "board/pic.h"
#include "bin/elf.h"
#include "kernel/paging.h"
#include "board/memory.h"
#include "board/cpu.h"
#include "lib/list.h"

typedef struct {
    ListNode head;
    Task task;
} TaskNode;

#define TASK_NODE(n)    ((TaskNode*)n)

#define TASKS_MAX           32
#define TID_KERNEL          0

#define PAGE_CODE       0x00
#define PAGE_DATA       0x01
#define PAGE_STACK      0x02

extern void task_run();

TaskStateSegment* TSS = (TaskStateSegment*)KERNEL_TSS_ADDRESS;
GDTEntry* tss_gdt_entry = (GDTEntry*)KERNEL_TSS_GDT_ENTRY;

//static Task TASKS[TASKS_MAX];

static ListNode* task_list;
static ListNode* current_task_list_node;

Task** current_task_ptr = (Task**)KERNEL_CURRENT_TASK_PAGE;
#define current_task (*current_task_ptr)

static uint32_t next_tid;

void tasks_init(){
    next_tid = 1;
    //memset(TASKS,0,sizeof(TASKS));
    task_list = NULL;
    current_task_list_node = NULL;
    current_task = NULL;

    tss_gdt_entry->accessed = 1;
    tss_gdt_entry->available = 0;
    tss_gdt_entry->dpl = 3;
    tss_gdt_entry->present = 1;
    tss_gdt_entry->read_write = 0;
    tss_gdt_entry->expand_down = 0;
    tss_gdt_entry->big = 1;
    tss_gdt_entry->code = 1;
    tss_gdt_entry->code_data_segment = 0;

    memset(TSS,0,sizeof(TaskStateSegment));

    TSS->ss0 = 0x10;
    TSS->esp0 = 0x2FEF;

    asm volatile(
        "\tmov $0x1b, %eax\n"
        "\tltr %ax\n"
    );
}

uint32_t tasks_current_tid(){
    return current_task ? current_task->tid : 0;
}

Task* tasks_current_task(void){
    return current_task;
}

static TaskNode* new_node(){
    TaskNode* task_node = heap_alloc(sizeof(TaskNode));
    memset(task_node,0,sizeof(TaskNode));
    return task_node;
}

static Task* get_next_free_task(){
    TaskNode* task_node = new_node();
    debug("List add 0:");debug_i(task_node,16);debug("\n");
    debug("List add 1:");debug_i((uint32_t)task_list,16);debug("\n");
    task_list = list_add(task_list, LIST_NODE(task_node));
    debug("List add 2:");debug_i(TASK_NODE(task_list)->task.tid,16);debug("\n");
    debug("List add 3:");debug_i(task_node,16);debug("\n");

    return &(task_node->task);
}

uint32_t tasks_new(Stream* exec_stream){
    VirtualAddress v_address;
    Task* task = get_next_free_task();

    if (!task){
        return 0;
    }
    if (task->tid != 0){
        debug("MAAAL\n");
        return 0;
    }
    memset(task,0,sizeof(Task));

    task->tid = next_tid++;
    task->status = TASK_STATUS_IDLE;
    debug("TASK - Allocating page directory\n");
    task->page_directory = paging_new_task_space();

    debug("TASK - Loading code into memory\n");
    task->cpu_state.eip = paging_load_code(exec_stream, task->page_directory);
    debug("TASK - Setting up task\n");

    task->cpu_state.cr3 = (uint32_t)task->page_directory;
    debug("TASK - Page directory for task:");debug_i(task->cpu_state.cr3,16);debug("\n");
    task->cpu_state.source_ss = 0x2B;
    v_address.page_dir_index = 1023;
    v_address.page_index = 1022;
    v_address.offset = 0xFFF;
    task->cpu_state.esp = v_address.address;
    task->cpu_state.source_esp = v_address.address;
    task->cpu_state.flags.privilege_level = 3;
    task->cpu_state.flags.reserved_1 = 1;
    task->cpu_state.flags.interrupt_enable = 1;
    debug("TASK - Program start:");debug_i(task->cpu_state.eip,16);debug("\n");

    return task->tid;
}

static Task* next_task(){
    if (current_task_list_node){
        current_task_list_node = current_task_list_node->next;
    }
    if (!current_task_list_node){
        current_task_list_node = task_list;
    }
    if (current_task_list_node){
        return &(TASK_NODE(current_task_list_node)->task);
    }
    return NULL;
}

static void remove_current_task(){
    ListNode* new_current = NULL;
    for (ListNode* t = task_list; t; t = t->next){
        if (t->next == LIST_NODE(current_task_list_node)){
            new_current = t;
            break;
        }
    }

    task_list = list_remove(LIST_NODE(task_list), LIST_NODE(current_task_list_node));
    debug("TASK - Task list after remove:");debug_i((uint32_t)task_list,16);debug("\n");
    heap_free(current_task_list_node);

    current_task_list_node = new_current;
}

void tasks_next(){
    current_task = next_task();
    debug("New task:");debug_i(current_task ? current_task->tid : 0,10);debug("\n");
}

void tasks_loop(){
    current_task = next_task();
    debug("TASK - Next task:");debug_i(current_task->tid,10);debug("\n");
    if (current_task && current_task->status != TASK_STATUS_NONE){
        current_task->status = TASK_STATUS_RUNNING;
        debug("TASK - Task ");debug_i(current_task->tid,10);debug(" set to run\n");

        task_run();

        debug("TASK - Task left cpu\n");
        debug_i(current_page_dir(),16);
        debug("\n");

        if (current_task->status != TASK_STATUS_NONE){
            debug("TASK - Task ");debug_i(current_task->tid,10);debug(" set idle\n");
            current_task->status = TASK_STATUS_IDLE;
        } else {
            debug("TASK - Removing task ");debug_i(current_task->tid,10);debug("\n");
            remove_current_task();
        }

    } else {
        debug("TASK - No tasks to run\n");
    }
    debug("TASK - Leave tasks loop\n");
}

void tasks_finish(uint32_t task_id, uint32_t exit_code){
    debug("TASK - Task finish:");debug_i(task_id,10);debug("\n");

    for (ListNode* task_node = task_list; task_node; task_node = task_node->next){
        if (TASK_NODE(task_node)->task.tid == task_id){
            Task* task = &(TASK_NODE(task_node)->task);
            debug("TASK - Releasing task ");debug_i(task_id,10);debug("\n");
            paging_release_task_space(task->page_directory);
            task->status = TASK_STATUS_NONE;
            //memset(task,0,sizeof(Task));
            debug("TASK - Task finished\n");
            break;
        }
    }
}

void tasks_update_current(InterruptFrame* frame){
    if (current_task){
        debug("TASK - Updating current task\n");
        memcpy(&(current_task->cpu_state), frame, sizeof(InterruptFrame));
    }
}
void* tasks_to_kernel_address (void* address){
    uint32_t physical_address = paging_physical_address(
        current_task->page_directory,
        address);
    return paging_to_kernel_space(physical_address);
}
