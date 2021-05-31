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
/*
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
*/
static void show_elf(Stream* stream){
    ElfHeader elf_header;
    ElfProgramHeader prg_header;
    ElfSectionHeader sec_header;
    uint32_t entries;
    uint32_t i;

    elf_read_header(stream, &elf_header);

    console_print("Magic number:");
    console_print(itoa(elf_header.magic_number,buff,16));
    console_print("\n");
    console_print("Binary type:");
    console_print(itoa(elf_header.bin_type,buff,10));
    console_print("\n");
    console_print("Arch:");
    console_print(elf_header.arch == 1 ? "32bit" : "64bit");
    console_print(",");
    console_print(elf_header.endianess == 1 ? "Little endian" : "Big Endian");console_print("\n");
    console_print("Header size:");
    console_print(itoa(elf_header.header_size,buff,10));
    console_print("\n");
    console_print("Program entry position:");
    console_print(itoa(elf_header.program_entry_position,buff,16));
    console_print("\n");
    console_print("Program header position:");
    console_print(itoa(elf_header.program_header_table_position,buff,10));
    console_print("\n");
    console_print("Program header size:");
    console_print(itoa(elf_header.program_header_table_entry_size,buff,10));
    console_print("\n");
    console_print("Section header position:");
    console_print(itoa(elf_header.section_header_table_position,buff,10));
    console_print("\n");
    console_print("Program header:\n");

    entries = elf_header.program_header_table_entry_count;

    for (i=0;i<entries;i++){
        elf_read_program_header(stream, &elf_header, i, &prg_header);
        console_print("  #");
        console_print(itoa(i,buff,10));
        console_print(", type:");
        console_print(itoa(prg_header.segment_type,buff,10));
        console_print(", Address:");
        console_print(itoa(prg_header.virtual_address,buff,16));
        console_print(", Offset:");
        console_print(itoa(prg_header.offset,buff,10));
        console_print(", Img.size:");
        console_print(itoa(prg_header.segment_file_size,buff,10));
        console_print(", Mem.size:");
        console_print(itoa(prg_header.segment_mem_size,buff,10));
        console_print(", Align:");
        console_print(itoa(prg_header.alignment,buff,10));
        console_print("\n");
    }

    console_print("Section header:\n");
    entries = elf_header.section_header_table_entry_count;

    for (i=0;i<entries;i++){
        elf_read_section_header(stream, &elf_header, i, &sec_header);
        console_print("  #");
        console_print(itoa(i,buff,10));
        console_print(", type:");
        if (sec_header.type <= 11){
            console_print(section_types[sec_header.type]);
        } else {
            console_print(itoa(sec_header.type,buff,10));
        }
        console_print(", name:");
        console_print(itoa(sec_header.name,buff,10));
        console_print(", Address:");
        console_print(itoa(sec_header.address,buff,16));
        console_print(", Size:");
        console_print(itoa(sec_header.size,buff,16));
        console_print("\n");
    }
}

void test_elf(){
    Ext2FileSystem* fs;
    Device* device;
    Stream* stream;

    device = device_find(DISK, 0);
    if (!device){
        console_print("Device not found\n");
        return;
    }
    fs = ext2_open(BLOCK_DEVICE(device));

    stream = ext2_file_stream_open(fs, "/test1.elf",0);

    if (!stream){
        ext2_close(fs);
        console_print("File not found\n");
        return;
    }
    console_print("Elf file /test1.elf open\n");
    show_elf(stream);

    stream_close(stream);
    ext2_close(fs);

}

void test_task(){
    Ext2FileSystem* fs;
    Device* device;
    Stream* stream;
    uint32_t task_id;

    console_print("Loading task in memory\n");

    device = device_find(DISK, 0);
    if (!device){
        console_print("Device not found\n");
        return;
    }
    fs = ext2_open(BLOCK_DEVICE(device));

    stream = ext2_file_stream_open(fs, "/test1.elf",0);

    task_id = tasks_new(stream);

    debug("New task:");debug_i(task_id,10);debug("\n");

    tasks_switch_to_task(task_id);
}
