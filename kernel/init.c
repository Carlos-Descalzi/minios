#include "board/console.h"
#include "board/pci.h"
#include "board/io.h"
#include "board/pit.h"
#include "board/pic.h"
#include "board/minfo.h"
#include "board/bda.h"
#include "board/ps2.h"
#include "kernel/task.h"
#include "kernel/paging.h"
#include "kernel/isr.h"
#include "kernel/device.h"
#include "lib/stdlib.h"
#include "lib/string.h"
#include "lib/heap.h"
#include "fs/ext2.h"
#include "misc/debug.h"

typedef struct {
    uint32_t total_ram;
    uint8_t biggest_region;
    uint64_t biggest_region_size;
    uint64_t biggest_region_address;
} MemData;

const char BANNER[] = "*************\n** MINI OS **\n*************\n";

static char buff[32];

static void dummy_handler();
static uint8_t show_memory_region(MemoryRegion* region, uint8_t, MemData* mem_data);
static void display_memory();
static uint16_t show_device(uint32_t number, uint8_t kind, Device* device, void* data);
static void check_e2fs();

extern void test_call();
extern void handle_gpf();
extern void devices_register();

void init(){
    debug("Kernel initializing\n");
    console_init();
    console_print(BANNER);
    console_gotoxy(0,4);
    console_print("PCI Devices:\n");

    isr_install(9, dummy_handler);
    isr_install(0x0D, handle_gpf);
    
    display_memory();
    paging_init();
    heap_init();

    device_init();
    devices_register();
    device_init_devices();
    console_print("devices initialized:\n");
    device_list(show_device,NULL);
    check_e2fs();
}
/*
static void show_pci_entry(uint8_t bus, uint8_t device, uint8_t func, PCIHeader* header){
    uint8_t header_type;
    int i;
    debug("Bus:");
    debug_i(bus,16);
    debug(" Dev:");
    debug_i(device,16);
    debug(" Func:");
    debug_i(func,10);
    debug(" Vendor:");
    debug_i(header->base.vendor_id,16);
    debug(" Device:");
    debug_i(header->base.device_id,16);
    debug(" Class:");
    debug_i(header->base.class,16);
    debug(" Subclass:");
    debug_i(header->base.subclass,16);
    if (header->base.header_type.mf){
        debug(" (MF)");
    }
    debug("\n");
    header_type = header->base.header_type.type;

    if (header_type == 0){
        for (i=0;i<6;i++){
            uint32_t address = header->type00.base_addresses[i];
            if(address){
                debug("\t");
                debug("Address ");
                debug_i(i,10);
                debug(" ");
                debug_i(address,16);
                if (address & 1){
                    debug(" (IO Port)");
                }
                debug("\n");
            }
        }
    } else if (header_type == 1){
        for (i=0;i<2;i++){
            uint32_t address = header->type01.base_addresses[i];
            if(address){
                debug("\t");
                debug("Address ");
                debug_i(i,10);
                debug(" ");
                debug_i((address & ~1),16);
                if (address & 1){
                    debug(" (IO Port)");
                }
                debug("\n");
            }
        }
    } else {
        debug("\tType 02\n");
    }
}
static void test_timer(){
    timer_count = 0;
    isr_install(8, timer_handler); // TODO: Remap PIC
    sti();
    while(timer_count < 3);
    cli();
}
*/

/*
static void INTERRUPT timer_handler(InterruptFrame* frame){
    console_print("Timer called ");
    timer_count++;
    console_print(utoa(timer_count,buff,10));
    console_put('\n');
    eoi();
}
*/
static void INTERRUPT dummy_handler(InterruptFrame* frame){
    sti();
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
    console_print(utoa(mem_data.total_ram,buff, 10));
    console_print(" Bytes free\n");

    console_print("Region #0 will be used for Kernel\n");
    console_print("Region #");
    console_print(utoa(mem_data.biggest_region,buff,10));
    console_print(" will be used for programs\n");

}

static uint8_t show_memory_region(MemoryRegion* region, uint8_t region_num, MemData* mem_data){

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

const struct {
    char* kind;
    char* description;
} KINDS[] = {
    {"video", "video"},
    {"ser", "serial"},
    {"hdd", "hard disk drive"},
    {"net", "network interface"},
    {"kbd", "keyboard"},
    {"mouse", "mouse"}
};

static uint16_t show_device(uint32_t number, uint8_t kind, Device* device, void* data){
    console_print("  * ");
    console_print(KINDS[kind].kind);
    console_print(itoa(device->instance_number,buff,10));
    console_print(" (");
    console_print(KINDS[kind].description);
    console_print(")\n");
    return 0;
}

static void check_e2fs(){
    Ext2FileSystem* fs;
    Device* device;

    device = device_find(HDD, 0);

    console_print("\n\nTesting ext2 filesystem for device kind HDD, instance 0 (hdd0)\n");

    if (!device){
        debug("Device not found\n");
    } else {
        fs = ext2_open(BLOCK_DEVICE(device));
        if (!fs){
            debug("Cannot open fs\n");
        } else {
            uint32_t inode;
            debug("INIT - Fs Ext2 open\n");

            inode = ext2_find_inode(fs, "/");
            console_print("Inode for / :");
            console_print(itoa(inode,buff,10));
            console_print("\n");

            inode = ext2_find_inode(fs, "/file1.txt");
            console_print("Inode for /file1.txt :");
            console_print(itoa(inode,buff,10));
            console_print("\n");

            inode = ext2_find_inode(fs, "/folder1");
            console_print("Inode for /folder1 :");
            console_print(itoa(inode,buff,10));
            console_print("\n");

            inode = ext2_find_inode(fs, "/folderx");
            console_print("Inode for /folderx :");
            console_print(itoa(inode,buff,10));
            console_print(" (not found, it's ok)\n");

            inode = ext2_find_inode(fs, "/folder1/file2.txt");
            console_print("Inode for /folder1/file2.txt :");
            console_print(itoa(inode,buff,10));
            console_print("\n");
        }

    }
}
