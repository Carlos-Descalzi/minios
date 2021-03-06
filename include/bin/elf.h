#ifndef _ELF_H_
#define _ELF_H_
/**
 * ELF binary format support
 **/

#include "lib/stdint.h"
#include "io/streams.h"

#define MAGIC_NUMBER_ELF    0x464C457F

#define ELF_ARCH_UNDEFINED  0x00
#define ELF_ARCH_SPARC      0x02
#define ELF_ARCH_X86        0x03
#define ELF_ARCH_POWERPC    0x14
#define ELF_ARCH_ARM        0x28
#define ELF_ARCH_SUPERH     0x2a
#define ELF_ARCH_IA64       0x32
#define ELF_ARCH_X86_64     0x3e
#define ELF_ARCH_AARCH64    0xb7

#define ELF_PH_PT_NULL      0x00
#define ELF_PH_PT_LOAD      0x01
#define ELF_PH_PT_DYNAMIC   0x02


typedef struct {
    uint32_t magic_number;
    uint8_t arch;
    uint8_t endianess;
    uint8_t elf_header_version;
    uint8_t os_abi;
    uint8_t padding[8];
    uint16_t bin_type;
    uint16_t instruction_set;
    uint32_t elf_version;
    uint32_t program_entry_position;
    uint32_t program_header_table_position;
    uint32_t section_header_table_position;
    uint32_t flags;
    uint16_t header_size;
    uint16_t program_header_table_entry_size;
    uint16_t program_header_table_entry_count;
    uint16_t section_header_table_entry_size;
    uint16_t section_header_table_entry_count;
    uint16_t section_names_index;
} ElfHeader;

#define ELF_PRG_HEADER_X    0x01
#define ELF_PRG_HEADER_W    0x02
#define ELF_PRG_HEADER_R    0x04

typedef struct {
    uint32_t segment_type;
    uint32_t offset;
    uint32_t virtual_address;
    uint32_t physical_address;
    uint32_t segment_file_size;
    uint32_t segment_mem_size;
    uint32_t flags;
    uint32_t alignment;
} ElfProgramHeader;

#define SHF_WRITE   0x1
#define SHF_ALLOC   0x2

#define ELF_SEC_HEADER_REL      0x09
#define ELF_SEC_HEADER_RELA     0x04
#define ELF_SEC_HEADER_PROGBITS 0x01

typedef struct {
    uint32_t name;
    uint32_t type;
    uint32_t flags;
    uint32_t address;
    uint32_t offset;
    uint32_t size;
    uint32_t link;
    uint32_t info;
    uint32_t alignment;
    uint32_t entry_size;
} ElfSectionHeader;

int32_t elf_read_header         (Stream* stream, ElfHeader* header);
int32_t elf_read_program_header (Stream* stream, ElfHeader* header, 
                                uint32_t headernum, ElfProgramHeader* prg_header);
int32_t elf_read_section_header (Stream* stream, ElfHeader* header, 
                                uint32_t headernum, ElfSectionHeader* section_header);
int32_t elf_read_section        (Stream* stream, ElfSectionHeader* section_header, uint8_t* dest);
int32_t elf_read_program_page   (Stream* stream, ElfProgramHeader* prg_header, 
                                void* dest, uint32_t blocknum, uint32_t page_size);
int32_t elf_read_section_page   (Stream* stream, ElfSectionHeader* sec_header, 
                                void* dest, uint32_t blocknum, uint32_t page_size);




#endif
