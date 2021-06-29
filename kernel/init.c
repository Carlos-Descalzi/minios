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

typedef struct {
    uint32_t total_ram;
    uint8_t biggest_region;
    uint64_t biggest_region_size;
    uint64_t biggest_region_address;
} MemData;

static char buff[32];

static void     test_timer          (void);
static void     test_isr            (InterruptFrame* frame, void* data);
static void     bsod                (InterruptFrame* frame, void* data);
static void     start_init          (void);

extern void     test_call           (void);
extern void     handle_gpf          (void);
extern void     devices_register    (void);
extern void     crash               (void);
extern void     test_elf            (void);
extern void     test_task           (void);
//static void     list_pci            (void);
static void     load_drivers        (void);

void init(){
    debug("Kernel initializing\n");
    console_init();
    console_print("*************\n** MINI OS **\n*************\n");
    console_gotoxy(0,4);

    //display_memory();

    memory_init();
    isr_init();


    paging_init();
    heap_init();

    fs_init();
    ext2_register_type();

    device_init();
    devices_register();
    device_init_devices();
    load_drivers();
    //test_elf();
    // while(1);

    pic_init();
    pit_init();
    
    tasks_init();
    syscall_init();
    trap_install(0xd,bsod, NULL);
    console_print("Kernel startup complete\n");
    //sti();

    test_task();
    //start_init();
    while(1);
}

static void test_isr(InterruptFrame* frame, void *data){
    console_print("ISR handler called\n");
    console_print("eax  :");console_print(itoa(frame->eax,buff,16));console_print("\n");
    console_print("eip  :");console_print(itoa(frame->eip,buff,16));console_print("\n");
    console_print("esp  :");console_print(itoa(frame->esp,buff,16));console_print("\n");
    console_print("ebp  :");console_print(itoa(frame->ebp,buff,16));console_print("\n");
    console_print("cs   :");console_print(itoa(frame->cs,buff,16));console_print("\n");
    console_print("cr3  :");console_print(itoa(frame->cr3,buff,16));console_print("\n");
    console_print("flags:");console_print(itoa(frame->flags.dwflags,buff,16));console_print("\n");
}

static void bsod(InterruptFrame* frame, void* data){
    int i;
    //console_gotoxy(0,0);
    console_color(CONSOLE_COLOR_BLUE << 4 | CONSOLE_COLOR_WHITE);
    //for(i=0;i<80*25;i++){console_put(' ');}
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


static void start_init(void){
    FileSystem* fs;
    Device* device;
    Stream* stream;
    uint32_t task_id;

    console_print("Loading program /init.elf ...\n");
    device = device_find(DISK, 0);
    if (!device){
        console_print("Device not found\n");
        return;
    }
    fs = fs_get_filesystem(device);
    stream = fs_file_stream_open(fs, "/init.elf",0);
    task_id = tasks_new(stream,0,NULL,0,NULL);
    stream_close(stream);
    debug("New task id:");debug_i(task_id,10);debug("\n");
    tasks_loop();
    console_print("System shutdown\n");
}
/*
static void show_pci(uint8_t a,uint8_t b, uint8_t c, PCIHeader* header, void*data){
    debug("Device:");
    debug(itoa(a,buff,16));
    debug("-");
    debug(itoa(b,buff,16));
    debug("-");
    debug(itoa(c,buff,16));
    debug(":");
    debug(itoa(header->base.header_type.type,buff,16));
    debug("\n");
    if (header->base.header_type.type == 0){
        for (int i=0;i<6;i++){
            if (header->type00.base_addresses[i]){
                debug("\tBase address:");
                debug(itoa(header->type00.base_addresses[i],buff,16));
                debug("\n");
            }
        }
    } else if (header->base.header_type.type == 1){
        debug("\tBase address:");
        debug(itoa(header->type01.memory_base,buff,16));
        debug("\n");
    } else if (header->base.header_type.type == 2){
        debug("\tBase address:");
        debug(itoa(header->type02.memory_base_0,buff,16));
        debug("\n");
    }
}

static void list_pci(void){
    pci_list_all_buses(show_pci, NULL);
}
*/
static void load_drivers(void){
    debug("Loading drivers\n");
    Device* device = device_find(DISK, 0);
    if (device){
        FileSystem* fs = fs_get_filesystem(device);
        if (fs){
            modules_load(fs, "/drivers/screen.elf");
            modules_load(fs, "/drivers/keyboard.elf");
            modules_load(fs, "/drivers/console.elf");
            debug("Drivers loaded\n");
        } else {
            debug("No fs\n");
        }
    } else {
        debug("No device\n");
    }
}
