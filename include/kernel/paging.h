#ifndef _PAGING_H_
#define _PAGING_H_

#include "lib/stdint.h"

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

#define PAGE_TYPE_READ          0
#define PAGE_TYPE_READ_WRITE    1

void                paging_init                 (void);
PageDirectoryEntry* paging_new_page_directory   (void);
uint32_t            paging_alloc_table          (PageDirectoryEntry* directory);
PageDirectoryEntry* paging_get_current_directory(void);
void                paging_set_directory        (PageDirectoryEntry* directory);


#endif
