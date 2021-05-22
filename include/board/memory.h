#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "lib/stdint.h"
/**
 * The following defines declare how 
 * lower 1Mb memory is used by kernel and
 * marks the beginning of physical user memory space
 **/
#define KERNEL_PAGE_DIR_ADDRESS             0x1000
#define KERNEL_PAGE_TABLE_ADDRESS           0x2000
#define USER_MEMORY_BITMAP_ADDRESS          0x2400
#define USER_MEMORY_START_ADDRESS           0x100000
#define KERNEL_UPPER_PAGE_TABLE_ADDRESS     0x30000
#define HEAP_MEMORY_START_ADDRESS           0x31000
#define HEAP_MEMORY_END_ADDRESS             0x5FFFF     // ~320 Kb

#define MEMORY_BLOCK_SIZE                   4096

void        memory_init         (void);
/**
 * Allocates a 4kb block
 **/
uint32_t    memory_alloc_block  (void);
/**
 * Frees a 4kb block
 **/
void        memory_free_block   (uint32_t block_address);

#endif
