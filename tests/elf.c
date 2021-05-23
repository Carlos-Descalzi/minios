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

static char buff[32];

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

void test_elf(){
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

    inodenum = ext2_find_inode(fs, "/test1.elf");
    
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
