#include "kernel/syscall.h"
#include "lib/stdint.h"
#include "kernel/isr.h"
#include "misc/debug.h"
#include "fs/ext2.h"
#include "kernel/task.h"
#include "fs/fs.h"

static void handle_syscall(InterruptFrame* f);

static void handle_read(InterruptFrame* f);
static void handle_write(InterruptFrame* f);
static void handle_open(InterruptFrame* f);
static void handle_close(InterruptFrame* f);
static void handle_exit(InterruptFrame* f);

void syscall_init(){
    isr_install(0x31, handle_syscall);
}

static void handle_syscall(InterruptFrame* f){
    debug("SYSCALL - Syscall called!\n");
    debug("\teax: ");debug_i(f->eax,16);debug("\n");
    debug("\tcr3: ");debug_i(f->cr3,16);debug("\n");
    debug("\tcs: ");debug_i(f->cs,16);debug("\n");
    debug("\tss: ");debug_i(f->source_ss,16);debug("\n");
    debug("\tesp: ");debug_i(f->source_esp,16);debug("\n");
    debug("\tflags: ");debug_i(f->flags.dwflags,16);debug("\n");
    debug("\teip: ");debug_i(f->eip,16);debug("\n");
    switch(f->eax){
        case 0x00:
            handle_read(f);
            break;
        case 0x01:
            handle_write(f);
            break;
        case 0x02:
            handle_open(f);
            break;
        case 0x03:
            handle_close(f);
            break;
        case 0x99:
            handle_exit(f);
            break;
        default:
            debug("\tUnknown system call:");debug_i(f->eax,16);debug("\n");
            break;
    }
}

static void handle_read(InterruptFrame* f){
    Stream *stream;
    Task* task = tasks_current_task();
    struct {
        uint8_t stream_num;
        uint8_t *buff;
        uint32_t size;
    } * read_data = ((void*)f->ebx);

    stream = task->streams[read_data->stream_num];

    f->ebx = (uint32_t)stream_read_bytes(stream, read_data->buff, read_data->size);
}
static void handle_write(InterruptFrame* f){
    Stream *stream;
    Task* task = tasks_current_task();
    struct {
        uint8_t stream_num;
        uint8_t *buff;
        uint32_t size;
    } * write_data = ((void*)f->ebx);

    stream = task->streams[write_data->stream_num];

    f->ebx = (uint32_t)stream_write_bytes(stream, write_data->buff, write_data->size);
}

#define RESOURCE_TYPE_RAW   0x00
#define RESOURCE_TYPE_FS    0x01

int next_stream_pos(Task *task){
    int i;
    for (i=0;i<32;i++){
        if (!task->streams[i]){
            return i;
        }
    }
    return -1;
}

static void handle_open(InterruptFrame* f){
    Device* device;
    int pos;
    Task* task = tasks_current_task();
    struct {
        uint8_t resource_type;
        uint8_t device_kind;
        uint8_t device_instance;
        uint8_t mode;
        const char path[1];
    } * open_data = ((void*)f->ebx);

    pos = next_stream_pos(task);

    if (pos >= 0){
        device = device_find(open_data->device_kind, open_data->device_instance);

        if (device){
            if (open_data->resource_type == RESOURCE_TYPE_RAW){
            } else {
                Ext2FileSystem* fs = fs_get_filesystem(device);
                task->streams[pos] = ext2_file_stream_open(fs, open_data->path, open_data->mode);
                f->ebx = pos;
            }
        } else {
            f->ebx = ((uint32_t)-2);
        }
    } else {
        f->ebx = ((uint32_t)-1);
    }
}
static void handle_close(InterruptFrame* f){
    Task* task = tasks_current_task();
    uint32_t stream_num = f->ebx;

    if (task->streams[stream_num]){
        stream_close(task->streams[stream_num]);
        task->streams[stream_num] = NULL;
        f->ebx = 0;
    } else {
        f->ebx = ((uint32_t)-1);
    }
}

static void handle_exit(InterruptFrame* f){
    uint32_t exit_code = f->ebx;
    debug("SYSCALL - Handling task exit, code:");debug_i(f->ebx,10);debug("\n");
    tasks_finish(tasks_current_tid(), exit_code);
    f->ebx = 0;
}
