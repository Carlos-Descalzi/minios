#ifndef _PAGING_H_
#define _PAGING_H_

#include "lib/stdint.h"
#include "io/streams.h"
#include "lib/params.h"
/**
 *
 * Task virtual memory layout
 * dir 0, pages 0 - 256: 1mb of lower ram - unused, readonly.
 * dir 0, page 257 - up to dir 20, page 47, free memory, used by memory mapping.
 * dir 20, page 48: executable code (this is actually the same place where linux places executable).
 * space between executable code and stack is all program memory.
 * dir 1023, page 1021: stack top.
 * dir 1023, page 1022: environment and parameters.
 * dir 1023, page 1023: interrupt service routines.
 **/

#define PAGE_SIZE               4096
#define PAGES_MAX               1024
#define PAGE_LAST               ((PAGES_MAX)-1)
/**
 * Page directory index, page table index, offset
 **/
#define mkvaddr(d,p,o)          (((d) << 22)|((p) << 12)|(o))

#define PAGING_TASK_ISR_PAGE    PAGE_LAST
#define PAGING_TASK_ENV_DIR     PAGE_LAST
#define PAGING_TASK_ENV_PAGE    (PAGE_LAST-1)
#define PAGING_TASK_STACK_DIR   PAGE_LAST
#define PAGING_TASK_STACK_PAGE  (PAGE_LAST-2)

#define PAGING_TASK_ENV_VADDR   mkvaddr(PAGING_TASK_ENV_DIR, PAGING_TASK_ENV_PAGE,0)
#define PAGING_TASK_STACK_VADDR mkvaddr(PAGING_TASK_STACK_DIR, PAGING_TASK_STACK_PAGE, 0xFFF)

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
        uint32_t addresssl: 12,
                 addressh: 20;
    };
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
uint32_t            paging_kernel_load_code     (Stream* stream);
/**
 * Allocates a kernel page
 **/
uint32_t            paging_alloc_kernel_page    (int rw);
/**
 * Releases a kernel page, releasing the associated space.
 **/
void                paging_free_kernel_page     (uint32_t virtual_address);

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
 * Maps a given physical address into a kernel page.
 **/
//void*               paging_to_kernel_space      (uint32_t physical_address, uint16_t size);

/**
 * Maps an address space from a given page directory
 * to kernel's page address space
 **/
void*               paging_task_to_kernel_space (PageDirectoryEntry* page_dir, 
                                                uint32_t virtual_address,
                                                uint16_t length);
/** 
 * Releases kernel pages which were referencing 
 * a physical address in use, in other words, it doesn't
 * release the associated memory.
 **/
//void                paging_free_mapped_kernel_pages
//                                                (uint32_t physical_address, uint16_t size);
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

/**
 * Maps a physical address + length into pages of a page directory
 * Returns the virtual address
 **/
uint32_t            paging_map_to_task          (PageDirectoryEntry* page_dir, 
                                                uint32_t address, 
                                                uint32_t length,
                                                uint8_t user);

#endif
