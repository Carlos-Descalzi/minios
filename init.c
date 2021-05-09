#include "console.h"
#include "pci.h"
#include "io.h"
#include "pit.h"
#include "pic.h"
#include "isr.h"
#include "stdlib.h"
#include "debug.h"
#include "task.h"
#include "minfo.h"
#include "heap.h"
#include "paging.h"

typedef struct {
    uint32_t total_ram;
    uint8_t biggest_region;
    uint64_t biggest_region_size;
    uint64_t biggest_region_address;
} MemData;

const char BANNER[] = "*************\n** MINI OS **\n*************\n";

static uint8_t timer_count;
static char buff[16];

static void timer_handler();
static void dummy_handler();
static void trap_handler();
static void show_pci_entry(uint8_t bus, uint8_t device, uint8_t func, HeaderBase* header);
static uint8_t show_memory_region(MemoryRegion* region, uint8_t, MemData* mem_data);
static void display_memory();
static void test_timer();

extern void test_call();
extern void handle_gpf();

void init(){
    uint32_t i =0;
    debug("Kernel initializing\n");
    console_init();
    console_print(BANNER);
    console_gotoxy(0,4);
    console_print("PCI Devices:\n");

    pci_list_all_buses(show_pci_entry);

    /*
    console_print("------\nTesting timer:\n");

    pit_init();
    pit_set_freq(1);

    test_timer();
    */
    isr_install(9, dummy_handler);
    isr_install(0x0D, handle_gpf);
    
    /*
    console_print("------\nInitializing task switcher:\n");

    tasks_init();
    sti();
    while(1){
        asm volatile("nop");
    }*/
    display_memory();
    paging_init();
    heap_init();

    pit_init();
    pit_set_freq(1);
    tasks_init();
    sti();

    while(1){}
}

static void show_pci_entry(uint8_t bus, uint8_t device, uint8_t func, HeaderBase* header){
    console_print("Bus:");
    console_print(itoa(bus,buff,16));
    console_print(" Dev:");
    console_print(itoa(device,buff,16));
    console_print(" Vendor:");
    console_print(itoa(header->vendor_id,buff,16));
    console_print(" Device:");
    console_print(itoa(header->device_id,buff,16));
    console_print(" Class:");
    console_print(itoa(header->class,buff,16));
    console_print(" Subclass:");
    console_print(itoa(header->subclass,buff,16));
    console_put('\n');
}

static void test_timer(){
    timer_count = 0;
    isr_install(8, timer_handler); // TODO: Remap PIC
    sti();
    while(timer_count < 3);
    cli();
}

static void INTERRUPT timer_handler(InterruptFrame* frame){
    console_print("Timer called ");
    timer_count++;
    console_print(itoa(timer_count,buff,10));
    console_put('\n');
    eoi();
}
static void INTERRUPT dummy_handler(InterruptFrame* frame){
    sti();
}
static void INTERRUPT trap_handler(InterruptFrame* frame){
    debug("general protection fault\n");
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

    minfo_iterate_regions((MemRegionVisitor)show_memory_region, &mem_data);
    console_print("Total Memory:");
    console_print(itoa(mem_data.total_ram,buff, 10));
    console_print(" Bytes free\n");

    console_print("Region #0 will be used for Kernel\n");
    console_print("Region #");
    console_print(itoa(mem_data.biggest_region,buff,10));
    console_print(" will be used for programs\n");

}

static uint8_t show_memory_region(MemoryRegion* region, uint8_t region_num, MemData* mem_data){

    console_print("Region#");
    console_print(itoa(region_num,buff,10));
    console_print(":");
    console_print(itoa(region->base_address,buff,16));
    console_print(" ");
    console_print(itoa(region->length,buff,10));
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
