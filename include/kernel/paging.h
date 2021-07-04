#ifndef _PAGING_H_
#define _PAGING_H_

#include "lib/stdint.h"
#include "io/streams.h"
#include "lib/params.h"

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

/**
 * Utility structure for handling virtual addresses
 **/
typedef union __attribute__((__packed__)){
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
/**
 * Returns the physical address for a page directory and virtual address
 **/
uint32_t            physical_address            (uint32_t page_dir, uint32_t address);
/**
 * Loads a given ELF binary into memory
 **/
uint32_t            paging_load_code            (Stream* stream, PageDirectoryEntry* dir);
/**
 *
 **/
uint32_t            paging_kenel_load_code      (Stream* stream);
/**
 * Returns a page directory configured for a new task
 **/
PageDirectoryEntry* paging_new_task_space       (void);
/**
 * Releases a page directory and all its allocated pages
 **/
void                paging_release_task_space   (PageDirectoryEntry* page_directory);
/**
 * Returns the physical address for a given page directory and virtual
 * address
 **/
uint32_t            paging_physical_address     (PageDirectoryEntry* page_dir, void *address);
/**
 * Maps a given physical address into a kernel page
 **/
void*               paging_to_kernel_space      (uint32_t physical_address);
/**
 * Writes arguments and enviroment in a page
 **/
void                paging_write_env            (PageDirectoryEntry* dir,
                                                TaskParams* args,
                                                TaskParams* env);
/**
 * Invalidates processor's TLB
 **/
void                paging_invalidate_cache     (void);
            
uint32_t            current_page_dir            (void);

#endif
