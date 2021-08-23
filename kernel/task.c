//#define NODEBUG
#include "kernel/task.h"
#include "kernel/isr.h"
#include "lib/string.h"
#include "lib/heap.h"
#include "misc/debug.h"
#include "board/pic.h"
#include "board/pit.h"
#include "bin/elf.h"
#include "kernel/paging.h"
#include "board/memory.h"
#include "board/cpu.h"
#include "lib/list.h"
#include "io/streamimpl.h"

typedef struct {
    ListNode head;
    Task task;
} TaskNode;

#define TASK_NODE(n)    ((TaskNode*)n)

typedef struct {
    ListNode head;
    Message message;
} MessageNode;

#define MSG_NODE(n)     ((MessageNode*)n)

TaskStateSegment* TSS = (TaskStateSegment*)KERNEL_TSS_ADDRESS;
GDTEntry* tss_gdt_entry = (GDTEntry*)KERNEL_TSS_GDT_ENTRY;
Task** current_task_ptr = (Task**)KERNEL_CURRENT_TASK_PAGE;

#define current_task    (*current_task_ptr)

static uint32_t next_tid;
static ListNode* task_list;
static ListNode* current_task_list_node;
static ListNode* io_wait_list;
static ListNode* sleep_list;

extern void     task_run                    (void);
static void     setup_console               (Task* task);
static void     remove_current_task         (void);
static void     check_io_wait_list          (void);
static int      check_pending_io_requests   (Task* task);
static void     answer_message              (Message* source_message, Message* target_message);
static void     do_send_message             (Message* message);
static void     handle_task_wait_condition  (uint32_t task_id, uint32_t exit_code);
static void     move_to_idle_list           (TaskNode* task_node);
static int      cnd_wait_tid                (Task* task);
static int      cnd_wait_msg_answer         (Task* task);
static int      cnd_wait_cnd_list           (Task* task);
static void     task_timer_callback         (uint64_t, void*);

void tasks_init(){
    next_tid = 1;
    task_list = NULL;
    io_wait_list = NULL;
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

    timer_add(122, task_timer_callback, NULL);
    //pit_add_callback(10, task_timer_callback, NULL);
}

uint32_t tasks_current_tid(){
    return current_task ? current_task->tid : 0;
}

Task* tasks_current_task(void){
    return current_task;
}
Task* tasks_get_task_by_tid(uint32_t tid){
    for (ListNode* n = task_list; n; n = n->next){
        if (TASK_NODE(n)->task.tid == tid){
            return &(TASK_NODE(n)->task);
        }
    }
    for (ListNode* n = io_wait_list; n; n = n->next){
        if (TASK_NODE(n)->task.tid == tid){
            return &(TASK_NODE(n)->task);
        }
    }
    return NULL;
}
    
static TaskNode* new_node(){
    TaskNode* task_node = heap_alloc(sizeof(TaskNode));
    memset(task_node,0,sizeof(TaskNode));
    return task_node;
}

static Task* get_next_free_task(){
    TaskNode* task_node = new_node();
    task_list = list_add(task_list, LIST_NODE(task_node));
    return &(task_node->task);
}

uint32_t tasks_new(
    uint32_t parent_task_id,
    Stream* exec_stream,
    TaskParams* args,
    TaskParams* env){
    
    Task* task = get_next_free_task();

    if (!task){
        return 0;
    }
    if (task->tid != 0){
        return 0;
    }
    memset(task,0,sizeof(Task));

    task->tid = next_tid++;
    task->parent_tid = parent_task_id;
    task->status = TASK_STATUS_IDLE;
    task->page_directory = paging_new_task_space();

    task->cpu_state.eip = paging_load_code(exec_stream, task->page_directory);

    task->cpu_state.cr3 = (uint32_t)task->page_directory;
    task->cpu_state.source_ss = 0x2B;
    task->args = args;
    task->env = env;

    uint32_t task_esp = mkvaddr(PAGING_TASK_STACK_DIR, PAGING_TASK_STACK_PAGE, 0xFFF);
    task->cpu_state.esp = task_esp;
    task->cpu_state.source_esp = task_esp;
    task->cpu_state.flags.privilege_level = 3;
    task->cpu_state.flags.reserved_1 = 1;
    task->cpu_state.flags.interrupt_enable = 1;
    setup_console(task);

    paging_write_env(task->page_directory, args, env);

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

static int check_pending_io_requests(Task* task){
    int any_pending = 0;
    for (int i = 0;i<4;i++){
        uint32_t status = task->io_requests[i].status;

        if (status == TASK_IO_REQUEST_PENDING){
            any_pending = 1;
        } else if (status == TASK_IO_REQUEST_DONE){
            task->cpu_state.ebx = task->io_requests[i].result;
            task->io_requests[i].status = TASK_IO_REQUEST_NONE;
        }
    }
    return any_pending;
}

static void check_io_wait_list(){
    for (ListNode* node = io_wait_list; node; node = node->next){

        int activate = 0;

        if (TASK_NODE(node)->task.status == TASK_STATUS_IOWAIT){
            if (!check_pending_io_requests(&(TASK_NODE(node)->task))){
                activate = 1;
            } 
        } else if (TASK_NODE(node)->task.status == TASK_STATUS_WAITCND){
            Task* task = &(TASK_NODE(node)->task);
            if (task->waitcond(task)){
                activate = 1;
            }
        }

        if (activate){

            move_to_idle_list(node);

            node = io_wait_list;

            if (!node){
                break;
            }
        }
    }
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

    paging_release_task_space(TASK_NODE(current_task_list_node)->task.page_directory);

    if (TASK_NODE(current_task_list_node)->task.args){
        heap_free(TASK_NODE(current_task_list_node)->task.args);
    }

    if (TASK_NODE(current_task_list_node)->task.env){
        heap_free(TASK_NODE(current_task_list_node)->task.env);
    }

    heap_free(current_task_list_node);
    current_task_list_node = new_current;
}

static void move_to_wait_list(){
    ListNode* next = current_task_list_node->next;
    task_list = list_remove(task_list, current_task_list_node);
    io_wait_list = list_add(io_wait_list, current_task_list_node);
    current_task_list_node = next ? next : task_list;
}

static void move_to_sleep_list(){
    ListNode* next = current_task_list_node->next;
    task_list = list_remove(task_list, current_task_list_node);
    sleep_list = list_add(sleep_list, current_task_list_node);
    current_task_list_node = next ? next : task_list;
}

int tasks_loop(){
    check_io_wait_list();
    
    current_task = next_task();

    if (current_task && current_task->status != TASK_STATUS_NONE){
        if (current_task->status == TASK_STATUS_IDLE){
            current_task->status = TASK_STATUS_RUNNING;

            task_run();


            if (current_task->status != TASK_STATUS_NONE){
                if (current_task->status == TASK_STATUS_RUNNING){
                    current_task->status = TASK_STATUS_IDLE;
                } else if (current_task->status == TASK_STATUS_IOWAIT
                    || current_task->status == TASK_STATUS_WAITCND){
                    move_to_wait_list();
                    current_task = NULL;
                } else if (current_task->status = TASK_STATUS_SLEEP){
                    move_to_sleep_list();
                    current_task = NULL;
                }
            } else {
                remove_current_task();
                current_task = NULL;
            }
            return 1;
        }

    } else {
        //debug("TASK - No tasks to run\n");
    }
    return 0;
}

void tasks_finish(uint32_t task_id, uint32_t exit_code){

    debug("TASK - Task finish:");debug_i(task_id,10);debug("\n");

    for (ListNode* task_node = task_list; task_node; task_node = task_node->next){
        if (TASK_NODE(task_node)->task.tid == task_id){
            Task* task = &(TASK_NODE(task_node)->task);
            task->status = TASK_STATUS_NONE;
            debug("TASK - Task finished\n");
            break;
        } 
    }
    handle_task_wait_condition(task_id, exit_code);
}

void tasks_update_current(InterruptFrame* frame){

    if (current_task){
        memcpy(&(current_task->cpu_state), frame, sizeof(InterruptFrame));
    }
}

void* tasks_to_kernel_address (void* address, uint16_t length){

    Task* task = current_task;

    return paging_task_to_kernel_space(task->page_directory, address, length);
}

void tasks_add_io_request(uint32_t type, uint32_t stream_num, uint8_t* buffer, uint32_t size){

    Task* task = current_task;

    for (int i=0;i<4;i++){
        if (task->io_requests[i].status == TASK_IO_REQUEST_NONE){
            memset(&(task->io_requests[i]),0,sizeof(IORequest));
            task->io_requests[i].status = TASK_IO_REQUEST_PENDING;
            task->io_requests[i].tid = current_task->tid;
            task->io_requests[i].type = type;
            task->io_requests[i].stream = stream_num;
            task->io_requests[i].target_buffer = buffer;
            task->io_requests[i].size = size;
            task->status = TASK_STATUS_IOWAIT;
            stream_read_async(
                task->streams[stream_num],
                &(task->io_requests[i])
            );
            break;
        }
    }
}

void tasks_wait_tid(uint32_t tid){
    Task* task = current_task;

    task->status = TASK_STATUS_WAITCND;
    task->waitcond = cnd_wait_tid;
    task->cond_data.int_data = tid;
}

int tasks_wait_conditions (WaitCondition* conditions){
    Task* task = current_task;

    task->status = TASK_STATUS_WAITCND;
    task->waitcond = cnd_wait_cnd_list;
    task->cond_data.ptr_data = conditions;
}

void tasks_send_message_sync(Message* message){

    Task* task = current_task;

    do_send_message(message);

    task->status = TASK_STATUS_WAITCND;
    task->waitcond = cnd_wait_msg_answer;
    task->cond_data.int_data = paging_physical_address(task->page_directory, message);
}

void tasks_send_message(Message* message){
    do_send_message(message);
}

int tasks_check_for_message(Message* message){

    Task* task = current_task;

    if (task->incoming_messages){

        MessageNode* message_node = MSG_NODE(task->incoming_messages);
        task->incoming_messages = list_remove(task->incoming_messages, LIST_NODE(message_node));

        message = paging_task_to_kernel_space(task->page_directory, message, sizeof(Message));
        answer_message(&(message_node->message), message);

        heap_free(message_node);

        return 0;
    }
    return -1;
}

int tasks_wait_message(Message* message){
    Task* task = current_task;

    if (tasks_check_for_message(message) < 0){
        task->status = TASK_STATUS_WAITCND;
        task->waitcond = cnd_wait_msg_answer;
        task->cond_data.ptr_data = message;
    } else {
        debug("Message already present, answered\n");
    }
    return 0;
}

void tasks_sleep (uint32_t secs, uint64_t nsecs){
    // TODO: for the moment only handle musec granularity
    current_task->sleep_nsecs = secs * 1000000000 + nsecs;
    current_task->status = TASK_STATUS_SLEEP;
}

static void do_send_message(Message* message){
    /**
     * get physical addresss of source message,
     * and place it on target task
     **/
    Task* task = current_task;

    MessageNode* message_node = heap_alloc(sizeof(MessageNode));

    message = paging_task_to_kernel_space( task->page_directory, (uint32_t) message, sizeof(Message));

    memcpy(&(message_node->message), message, sizeof(Message));

    debug("TASKS - send message to ");debug_i(message->target,10);debug("\n");

    Task* target_task = tasks_get_task_by_tid(message->target);

    if (target_task){
        target_task->incoming_messages = list_add(
            target_task->incoming_messages,
            LIST_NODE(message_node)
        );
    } else {
        debug("Task not found\n");
    }
}

static void answer_message(Message* source_message, Message* target_message){
    memcpydw(target_message, source_message, sizeof(Message) / 4);
}

static void setup_console(Task* task){
    // TODO Rework console stuff
    task->console = CHAR_DEVICE(device_find(TERM,0));
    if (task->console){
        // stdin
        task->streams[0] = device_stream_open(DEVICE(task->console),STREAM_READ);
        // stdout
        task->streams[1] = device_stream_open(DEVICE(task->console),STREAM_WRITE);
        // stderr
        task->streams[2] = device_stream_open(DEVICE(task->console),STREAM_WRITE);
    } else {
        debug("Warning: No console\n");
    }
}

int tasks_count (void){
    return 
        list_size(task_list)
        + list_size(io_wait_list);
}

void tasks_iter_tasks(TaskVisitor visitor, void* data){
    for (ListNode* n = task_list; n; n = n->next){
        if (visitor(&(TASK_NODE(n)->task), data)){
            break;
        }
    }
    for (ListNode* n = io_wait_list; n; n = n->next){
        if (visitor(&(TASK_NODE(n)->task), data)){
            break;
        }
    }
}

/** 
 * Wait a given task to finish
 **/
static int cnd_wait_tid(Task* task){
    /**
     * This is handled by process exit, not really a condition function
     **/
    return 0;
}

/**
 * Waits for any of a list of conditions to apply
 **/
static int cnd_wait_cnd_list (Task* task){

    WaitCondition* conditions = task->cond_data.ptr_data;

    int applies = 0;
  
    for (int i=0;i<conditions->n_conditions;i++){
        
        switch(conditions->items[i].cond_type){
            case COND_TYPE_FD:{
                // wait a file descriptor to have data available
                Stream* stream = task->streams[conditions->items[i].fd];
                if (stream && stream_available(stream)){
                    applies = 1;
                    break;
                }
                break;
            }
            case COND_TYPE_MSG:
                // Wait to have message avaiable
                if (task->incoming_messages){
                    applies = 1;
                    break;
                }
                break;
        }
    }
    if (applies){
        heap_free(task->cond_data.ptr_data);
        return 1;
    }
    return 0;
}

/**
 * Wait an answer for a message
 **/
static int cnd_wait_msg_answer(Task* task){
    debug("TASKS - wait message answer from ");debug_i(task->tid,10);
    debug(",");debug_i(task->incoming_messages,16);
    debug(",");debug_i(list_size(task->incoming_messages),16);
    debug("\n");


    for (ListNode* node = task->incoming_messages; node; node = node->next){
        Message* waiting_msg = paging_task_to_kernel_space(
            task->page_directory,
            (uint32_t) task->cond_data.ptr_data,
            sizeof(Message)
        );
        uint32_t source = waiting_msg->source;
        uint32_t target = waiting_msg->target;

        if (source == 0 
            && target == 0){
            // receive any message
            task->cpu_state.ebx = 0;
            answer_message(&(MSG_NODE(node)->message), waiting_msg);
            task->incoming_messages = list_remove(task->incoming_messages, node);
            heap_free(node);
            return 1;
        } else {
            // receive a message from a specific source to a specific target.
            debug("TASKS - answer message for task\n");
            if (target == MSG_NODE(node)->message.source 
                && source == MSG_NODE(node)->message.target){
                task->cpu_state.ebx = 0;
                answer_message(&(MSG_NODE(node)->message), waiting_msg);
            } else {
                debug("Wrong source - target: ");
            }
            task->incoming_messages = list_remove(task->incoming_messages, node);
            heap_free(node);

            return 1;
        }
    }
    return 0;
}
static void handle_task_wait_condition  (uint32_t task_id, uint32_t exit_code){
    for (ListNode* node = io_wait_list; node; node = node->next){

        if (TASK_NODE(node)->task.status == TASK_STATUS_WAITCND
            && TASK_NODE(node)->task.waitcond == cnd_wait_tid
            && TASK_NODE(node)->task.cond_data.int_data == task_id){

            Task* task = &(TASK_NODE(node)->task);
            task->cpu_state.ebx = exit_code;

            move_to_idle_list(node);

            node = io_wait_list;

            if (!node){
                break;
            }
        }
    }

}

static void move_to_idle_list(TaskNode* task_node){
    task_node->task.status = TASK_STATUS_IDLE;
    io_wait_list = list_remove(io_wait_list, task_node);
    task_list = list_add(task_list, task_node);
}

static void task_timer_callback (uint64_t ticks, void* data){
    // TODO: fix timings
    if (current_task){
        current_task->ticks+=10;
    }
    //debug("Tick\n");
    for (ListNode* n = sleep_list; n; n = n->next){
        //debug("TASK sleep tick\n");
        TASK_NODE(n)->task.sleeping_nsecs += 300000000;
        if (TASK_NODE(n)->task.sleeping_nsecs >= TASK_NODE(n)->task.sleep_nsecs){

            TASK_NODE(n)->task.status = TASK_STATUS_IDLE;

            sleep_list = list_remove(sleep_list, n);
            task_list = list_add(task_list, n);
            n = sleep_list;
            if (!n){
                break;
            }
        }
    }
}
