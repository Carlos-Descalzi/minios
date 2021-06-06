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
#include "fs/ext2.h"
#include "kernel/syscall.h"

typedef struct {
    uint32_t total_ram;
    uint8_t biggest_region;
    uint64_t biggest_region_size;
    uint64_t biggest_region_address;
} MemData;

static char buff[32];

const struct {
    char* kind;
    char* description;
} KINDS[] = {
    {"video", "video"},
    {"ser", "serial"},
    {"disk", "disk drive"},
    {"net", "network interface"},
    {"kbd", "keyboard"},
    {"mouse", "mouse"}
};

static void     dummy_handler       (void);
static uint8_t  show_memory_region  (MemoryRegion* region, uint8_t, void* data);
static void     display_memory      (void);
static uint16_t show_device         (uint32_t number, uint8_t kind, Device* device, void* data);
static void     test_timer          (void);
static void     test_isr            (InterruptFrame frame);
static void     handle_keyboard     (InterruptFrame frame);

extern void     test_call           (void);
extern void     handle_gpf          (void);
extern void     devices_register    (void);
extern void     crash               (void);
extern void     check_e2fs          (void);
extern void     test_elf            (void);
extern void     test_task           (void);

void init(){
    debug("Kernel initializing\n");
    console_init();
    console_print("*************\n** MINI OS **\n*************\n");
    console_gotoxy(0,4);

    display_memory();

    memory_init();
    isr_init();

    //pit_init();
    //pic_init();
    //test_timer();
    //sti();
    //isr_install(PIC_IRQ_BASE+0x01, handle_keyboard);
    //while(1);

    paging_init();
    console_print("Testing ISR\n");
    isr_install(0x30, test_isr);
    asm volatile("int $0x30");
    heap_init();
    device_init();
    devices_register();
    device_init_devices();
    //test_elf();
    tasks_init();
    syscall_init();
    asm volatile("int $0x31");

    test_task();
    
    //check_e2fs();
    /*
    */
    //console_print("Tested ISR\n");
    //crash();
    
    //debug("Initializing PIC\n");

    /*
    isr_install(0x40, dummy_handler);
    isr_install(0x0D, handle_gpf);

    asm volatile ("int $0x40");
    */
    //sti();
    //isr_install(0x20, dummy_handler);
    //isr_install(0x21, dummy_handler);
    //while(1);

    /*
    
    display_memory();
    paging_init();

    devices_register();
    device_init_devices();
    console_print("devices initialized:\n");
    device_list(show_device,NULL);
    */
}
static void handle_keyboard(InterruptFrame frame){
    cli();
    console_print("Key pressed\n");
    inb(0x64);
    inb(0x60);
    pic_eoi();
    sti();
}

static void test_isr(InterruptFrame frame){
    console_print("ISR handler called\n");
    console_print("eax  :");console_print(itoa(frame.eax,buff,16));console_print("\n");
    console_print("eip  :");console_print(itoa(frame.eip,buff,16));console_print("\n");
    console_print("esp  :");console_print(itoa(frame.esp,buff,16));console_print("\n");
    console_print("ebp  :");console_print(itoa(frame.ebp,buff,16));console_print("\n");
    console_print("cs   :");console_print(itoa(frame.cs,buff,16));console_print("\n");
    console_print("cr3  :");console_print(itoa(frame.cr3,buff,16));console_print("\n");
    console_print("flags:");console_print(itoa(frame.flags.dwflags,buff,16));console_print("\n");
}

static int timer_count;

static void timer_handler(InterruptFrame frame){
    timer_count++;
    console_print("Timer called, IRQ: ");
    console_print(utoa(pic_get_irq(),buff,16));
    console_put('\n');
    pic_eoi();
}

static void test_timer(){
    console_print("Testing timer\n");
    timer_count = 0;
    isr_install(0x20, timer_handler); 
    sti();
    while(timer_count < 3);
    cli();
    isr_install(0x20, NULL); 
}

void bsod(){
    int i;
    console_gotoxy(0,0);
    console_color(CONSOLE_COLOR_BLUE << 4 | CONSOLE_COLOR_WHITE);
    for(i=0;i<80*25;i++){console_put(' ');}
    console_gotoxy(30,12);
    console_print("General Protection Fault");
    console_gotoxy(30,14);
    console_print("  *** Te re cabio ***");
}

static void display_memory(){
    MemData mem_data = {0,0,0};
    console_print("------\nChecking RAM:\n");

    minfo_iterate_regions(show_memory_region, &mem_data);
    console_print("Total Memory:");
    console_print(utoa(mem_data.total_ram,buff, 10));
    console_print(" Bytes free\n");

    console_print("Region #0 will be used for Kernel\n");
    console_print("Region #");
    console_print(utoa(mem_data.biggest_region,buff,10));
    console_print(" will be used for programs\n");

}

static uint8_t show_memory_region(MemoryRegion* region, uint8_t region_num, void* data){
    MemData* mem_data = (MemData*)data;

    console_print("Region#");
    console_print(utoa(region_num,buff,10));
    console_print(":");
    console_print(utoa(region->base_address,buff,16));
    console_print(" ");
    console_print(utoa(region->length,buff,10));
    if (region->type == MEM_TYPE_FREE){
        mem_data->total_ram+=region->length;
        console_print(" (Free)\n");
        if (region->length > mem_data->biggest_region_size){
            mem_data->biggest_region = region_num;
            mem_data->biggest_region_address = region->base_address;
            mem_data->biggest_region_size = region->length;
        }
    } else {
        console_print(" (Reserved)\n");
    }
    return 0;
}
/*
static void check_bda(){
    int i;
    console_print("------\nSerial Ports:\n");
    for (i=0;i<4;i++){
        if (BDA->com_ports[i]){
            console_print("Serial port ");
            console_print(utoa(i,buff,10));
            console_print(" ");
            console_print(utoa(BDA->com_ports[i],buff,16));
            console_print("\n");
        }
    }
}
static void check_ps2(){
    PS2Port ps2;
    console_print("------\nPS/2:\n");

    ps2_get_status(&ps2);

    console_print("Controller: ");
    if (ps2.controller_present){
        console_print("Ok\n");
    } else {
        console_print("Not present\n");
    }
    console_print("Port 1: ");
    if (ps2.port1_present){
        console_print("Ok\n");
    } else {
        console_print("Not present\n");
    }
    console_print("Port 2: ");
    if (ps2.port2_present){
        console_print("Ok\n");
    } else {
        console_print("Not present\n");
    }
}
*/

static uint16_t show_device(uint32_t number, uint8_t kind, Device* device, void* data){
    console_print("  * ");
    console_print(KINDS[kind].kind);
    console_print(itoa(device->instance_number,buff,10));
    console_print(" (");
    console_print(KINDS[kind].description);
    console_print(")\n");
    return 0;
}

