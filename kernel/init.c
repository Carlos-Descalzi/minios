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
#include "fs/ext2.h"
#include "misc/debug.h"
#include "bin/elf.h"

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
static void     check_e2fs          (void);
static void     test_timer          (void);
static void     test_isr            (InterruptFrame frame);
static void     handle_keyboard     (InterruptFrame frame);
static void     test_elf            (void);

extern void     test_call           (void);
extern void     handle_gpf          (void);
extern void     devices_register    (void);
extern void     crash               (void);

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
    check_e2fs();
    test_elf();

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
    isr_install(0x20, timer_handler); // TODO: Remap PIC
    sti();
    while(timer_count < 3);
    cli();
    isr_install(0x20, NULL); // TODO: Remap PIC
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

static void check_e2fs(){
    Ext2FileSystem* fs;
    Device* device;
    char buffer[1024];
    memset(buffer,0,1024);

    device = device_find(DISK, 0);

    console_print("\n\nTesting ext2 filesystem for device kind DISK, instance 0 (disk0)\n");

    if (!device){
        debug("Device not found\n");
    } else {
        fs = ext2_open(BLOCK_DEVICE(device));
        if (!fs){
            debug("Cannot open fs\n");
        } else {
            uint32_t inodenum;
            Ext2Inode inode;
            debug("INIT - Fs Ext2 open\n");

            inodenum = ext2_find_inode(fs, "/");
            console_print("Inode for / :");
            console_print(itoa(inodenum,buff,10));
            console_print("\n");

            inodenum = ext2_find_inode(fs, "/file1.txt");
            console_print("Inode for /file1.txt :");
            console_print(itoa(inodenum,buff,10));
            console_print("\n");

            inodenum = ext2_find_inode(fs, "/folder1");
            console_print("Inode for /folder1 :");
            console_print(itoa(inodenum,buff,10));
            console_print("\n");

            inodenum = ext2_find_inode(fs, "/folderx");
            console_print("Inode for /folderx :");
            console_print(itoa(inodenum,buff,10));
            console_print(" (not found, it's ok)\n");

            inodenum = ext2_find_inode(fs, "/folder1/file2.txt");
            console_print("Inode for /folder1/file2.txt :");
            console_print(itoa(inodenum,buff,10));
            console_print("\n");

            ext2_load_inode(fs, inodenum, &inode);
            console_print("Contents:");
            ext2_load(fs, &inode, buffer);
            console_print(buffer);
            console_print("\n");
        }

    }
}
const char* section_types[] = {
    "NULL    ",
    "PROGBITS",
    "SYMTAB  ",
    "STRTAB  ",
    "RELA    ",
    "HASH    ",
    "DYNAMIC ",
    "NOTE    ",
    "NOBITS  ",
    "REL     ",
    "SHLIB   ",
    "DYNSYM  "
};
static const char* get_section_name(ElfHeader* header, ElfSectionHeader* sec_header){
    int i;
    if (sec_header->name){
        ElfSectionHeader* section =(ElfSectionHeader*) 
            (((void*)header) + header->section_header_table_position 
             + (header->section_names_index 
                * header->section_header_table_entry_size));
        return (const char*) (((void*)header) + section->offset + sec_header->name);
    }
    return NULL;
}
static void show_elf(void* elf,uint32_t fsize){
    ElfHeader* elf_header = (ElfHeader*)elf;
    ElfProgramHeader* prg_header = (ElfProgramHeader*)(((void*)elf)+ elf_header->program_header_table_position);
    ElfSectionHeader* sec_header = (ElfSectionHeader*)(((void*)elf) + elf_header->section_header_table_position);
    uint32_t entries;
    uint32_t size;
    uint32_t i;
    console_print("Magic number:");
    console_print(itoa(elf_header->magic_number,buff,16));
    console_print("\n");
    console_print("Binary type:");
    console_print(itoa(elf_header->bin_type,buff,10));
    console_print("\n");
    console_print("Arch:");
    console_print(elf_header->arch == 1 ? "32bit" : "64bit");
    console_print(",");
    console_print(elf_header->endianess == 1 ? "Little endian" : "Big Endian");console_print("\n");
    console_print("Header size:");
    console_print(itoa(elf_header->header_size,buff,10));
    console_print("\n");
    console_print("Program entry position:");
    console_print(itoa(elf_header->program_entry_position,buff,16));
    console_print("\n");
    console_print("Program header position:");
    console_print(itoa(elf_header->program_header_table_position,buff,10));
    console_print("\n");
    console_print("Program header size:");
    console_print(itoa(elf_header->program_header_table_entry_size,buff,10));
    console_print("\n");
    console_print("Section header position:");
    console_print(itoa(elf_header->section_header_table_position,buff,10));
    console_print("\n");
    console_print(itoa((uint32_t)elf_header,buff,16));
    console_print("\n");

    console_print("Program header:");
    console_print(itoa((uint32_t)prg_header,buff,16));
    console_print("\n");

    entries = elf_header->program_header_table_entry_count;
#define next_prg_header(h,ph) \
    ((ElfProgramHeader*)(((void*)ph)+(h->program_header_table_entry_size)))

    for (i=0;i<entries;i++,prg_header = next_prg_header(elf_header, prg_header)){
        console_print("  #");
        console_print(itoa(i,buff,10));
        console_print(", type:");
        console_print(itoa(prg_header->segment_type,buff,10));
        console_print(", Address:");
        console_print(itoa(prg_header->virtual_address,buff,16));
        console_print(", Offset:");
        console_print(itoa(prg_header->offset,buff,10));
        console_print(", Img.size:");
        console_print(itoa(prg_header->segment_file_size,buff,10));
        console_print(", Mem.size:");
        console_print(itoa(prg_header->segment_mem_size,buff,10));
        console_print(", Align:");
        console_print(itoa(prg_header->alignment,buff,10));
        console_print("\n");
    }

#define next_sec_header(h,sh) \
    ((ElfSectionHeader*)(((void*)sh)+(h->section_header_table_entry_size)))

    console_print("Section header:");
    console_print(itoa((uint32_t)sec_header,buff,16));
    console_print("\n");
    entries = elf_header->section_header_table_entry_count;

    for (i=0;i<entries;i++,sec_header = next_sec_header(elf_header, sec_header)){
        console_print("  #");
        console_print(itoa(i,buff,10));
        console_print(", type:");
        if (sec_header->type <= 11){
            console_print(section_types[sec_header->type]);
        } else {
            console_print(itoa(sec_header->type,buff,10));
        }
        console_print(", name:");
        console_print(get_section_name(elf_header, sec_header));
        console_print(", Address:");
        console_print(itoa(sec_header->address,buff,16));
        console_print(", Size:");
        console_print(itoa(sec_header->size,buff,16));
        console_print("\n");
    }

}

static void test_elf(){
    uint32_t inodenum;
    Ext2Inode inode;
    Ext2FileSystem* fs;
    Device* device;

    device = device_find(DISK, 0);
    if (!device){
        console_print("Device not found\n");
        return;
    }
    fs = ext2_open(BLOCK_DEVICE(device));

    inodenum = ext2_find_inode(fs, "/test.elf");
    
    if (!inodenum){
        console_print("inode not found\n");
    } else {
        char* buff = heap_alloc(inode.size);
        console_print("Loading elf file\n");
        ext2_load_inode(fs, inodenum, &inode);
        ext2_load(fs, &inode, buff);
        show_elf(buff,inode.size);
    }
    ext2_close(fs);
}
