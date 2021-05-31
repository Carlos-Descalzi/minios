#ifndef _PAGING_H_
#define _PAGING_H_

#include "lib/stdint.h"
#include "io/streams.h"

#define PAGE_SIZE               4096

typedef struct {
    uint32_t 
        present: 1,
        read_write: 1,
        user_supervisor: 1,
        write_through: 1,
        cache_disabled: 1,
        accessed: 1,
        unused_1:1,
        page_size:1,
        unused_2: 1,
        user_data: 3,
        page_table_address: 20;
} PageDirectoryEntry;

typedef struct {
    uint32_t 
        present: 1,
        read_write: 1,
        user_supervisor: 1,
        write_through: 1,
        cache_disabled: 1,
        accessed: 1,
        dirty: 1,
        unused_1:1,
        global: 1,
        user_data:3,
        physical_page_address: 20;
} PageTableEntry;

typedef union {
    struct {
        uint32_t offset:12,
                 page_index:10,
                 page_dir_index:10;
    };
    uint32_t address;
    void* paddress;
} VirtualAddress;

#define PAGE_TYPE_READ          0
#define PAGE_TYPE_READ_WRITE    1

void                paging_init                 (void);
PageDirectoryEntry* paging_new_page_directory   (uint8_t user_supervisor);
uint32_t            paging_alloc_table          (PageDirectoryEntry* directory);
PageDirectoryEntry* paging_get_current_directory(void);
void                paging_set_directory        (PageDirectoryEntry* directory);
/**
 * Returns the physical address for a page directory and virtual address
 **/
uint32_t            physical_address            (uint32_t page_dir, uint32_t address);

void                paging_load_code            (Stream* stream, PageTableEntry* start);


#endif
