#include "bin/elf.h"
#include "misc/debug.h"

int32_t elf_read_header(Stream* stream, ElfHeader* header){
    if (!stream || stream_read_bytes(stream, (uint8_t*)header, sizeof(ElfHeader)) < 0){
        return -1;
    }

    return 0;
}

int32_t elf_read_program_header(Stream* stream, ElfHeader* header, 
                                uint32_t headernum, ElfProgramHeader* prg_header){
    uint32_t pos;

    if (!stream || !header || headernum >= header->program_header_table_entry_count){
        return -1;
    }

    pos = header->program_header_table_position
        + header->program_header_table_entry_size * headernum;

    if (stream_seek(stream, pos) < 0){
        return -1;
    }

    if (stream_read_bytes(stream, (uint8_t*)prg_header, sizeof(ElfProgramHeader)) <0){
        return -1;
    }

    return 0;
}

int32_t elf_read_section_header(Stream* stream, ElfHeader* header, 
                                uint32_t headernum, ElfSectionHeader* section_header){

    uint32_t pos;

    if (!stream || !header || headernum >= header->section_header_table_entry_count){
        return -1;
    }

    pos = header->section_header_table_position
        + header->section_header_table_entry_size * headernum;

    if (stream_seek(stream, pos) < 0){
        return -1;
    }

    if (stream_read_bytes(stream, (uint8_t*)section_header, sizeof(ElfSectionHeader)) < 0){
        return -1;
    }

    return 0;
}

int32_t elf_read_section(Stream* stream, ElfSectionHeader* section_header, uint8_t* dest){

    if (!stream || !section_header){
        return -1;
    }

    stream_seek(stream, section_header->offset);
    stream_read_bytes(stream, dest, section_header->size);

    return 0;
}

int32_t elf_read_program(Stream* stream, ElfProgramHeader* prg_header, uint8_t* dest){

    if (!stream || !prg_header){
        return -1;
    }

    stream_seek(stream, prg_header->offset);
    stream_read_bytes(stream, dest, prg_header->segment_file_size);

    return 0;
}
int32_t elf_read_program_page(Stream* stream, ElfProgramHeader *prg_header, 
        void* dest, uint32_t blocknum, uint32_t page_size){
    if (!stream || !prg_header){
        return -1;
    }
    debug("10\n");
    stream_seek(stream, prg_header->offset + blocknum * page_size);
    debug("11\n");
    stream_read_bytes(stream, dest, page_size);
    debug("12\n");

    return 0;
}
