#ifndef _ELF_H_
#define _ELF_H_

#include "lib/stdint.h"

#define ELF_ARCH_UNDEFINED  0x00
#define ELF_ARCH_SPARC      0x02
#define ELF_ARCH_X86        0x03
#define ELF_ARCH_POWERPC    0x14
#define ELF_ARCH_ARM        0x28
#define ELF_ARCH_SUPERH     0x2a
#define ELF_ARCH_IA64       0x32
#define ELF_ARCH_X86_64     0x3e
#define ELF_ARCH_AARCH64    0xb7

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

typedef struct {
    uint32_t segment_type;
    uint32_t file_offset;
    uint32_t virtual_address;
    uint32_t reserved;
    uint32_t segment_file_size;
    uint32_t segment_mem_size;
    uint32_t flags;
    uint32_t alignment;
} ElfProgramHeader;

#endif
