#include "board/console.h"
#include "board/pci.h"
#include "board/io.h"
#include "board/pit.h"
#include "board/pic.h"
#include "board/minfo.h"
#include "board/bda.h"
#include "board/ps2.h"
#include "board/memory.h"
#include "kernel/task.h"
#include "kernel/paging.h"
#include "kernel/isr.h"
#include "kernel/device.h"
#include "lib/stdlib.h"
#include "lib/string.h"
#include "lib/heap.h"
#include "misc/debug.h"
#include "bin/elf.h"
#include "fs/fs.h"
#include "fs/ext2.h"
#include "kernel/syscall.h"
#include "kernel/modules.h"
#include "lib/params.h"
#include "board/rtc.h"
#include "kernel/timer.h"

static void     bsod                (InterruptFrame* frame, void* data);
static void     start_init          (void);
static void     load_program        (FileSystem* fs, const char* path, const char* name);

extern void     ide_register        (void);     // FIXME

void init(){
    debug("Kernel initializing\n");
    console_init();

    memory_init();
    isr_init();

    paging_init();
    heap_init();

    fs_init();
    ext2_register_type();

    device_init();
    ide_register();
    device_init_devices();

    pic_init();
    pit_init();
    rtc_init();

    timer_init();
    
    tasks_init();
    syscall_init();
    trap_install(0xd,bsod, NULL);
    console_print("Kernel startup complete\n");

    start_init();
}

static void bsod(InterruptFrame* frame, void* data){
    console_color(CONSOLE_COLOR_BLUE << 4 | CONSOLE_COLOR_WHITE);
    console_gotoxy(30,11);
    console_print("+------------------------+");
    console_gotoxy(30,12);
    console_print("|      Exception 0xd     |");
    console_gotoxy(30,13);
    console_print("|General Protection Fault|");
    console_gotoxy(30,14);
    console_print("|  *** Te re cabio ***   |");
    console_gotoxy(30,15);
    console_print("+------------------------+");
    asm volatile("hlt");
}

static void load_program(FileSystem* fs, const char* path, const char* name){
    console_print("Loading ");
    console_print(path);
    console_print("\n");
    
    Stream* stream = fs_open_stream_path(fs, path, O_RDONLY);

    char* args[]  = { (char*) name };
    
    uint32_t task_id = tasks_new(
        0,
        stream,
        task_params_from_char_array(1,args),
        NULL
    );
    
    stream_close(stream);
    
    debug("New task id:");debug_i(task_id,10);debug("\n");
}

static void start_init(void){

    Device* device = device_find(DISK, 0);

    if (!device){
        console_print("Device not found\n");
        return;
    }

    FileSystem* fs = fs_get_filesystem(BLOCK_DEVICE(device));

    load_program(fs, "/bin/init.elf", "init");

    sti();
    while (1){
        if (!tasks_loop()){
            asm volatile("hlt");
        }
    }
    console_print("System shutdown\n");
}
