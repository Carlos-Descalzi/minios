#include "heap.h"
#include "stdint.h"
#include "debug.h"
#include "stdlib.h"

#define MEMORY_START    0x30000
#define MEMORY_END      0x5FFFF     // ~320 Kb

typedef struct MemoryBlock MemoryBlock;

typedef struct {
    uint32_t used:8,
             size:24;
    MemoryBlock* next;
} BlockHeader;

struct MemoryBlock {
    BlockHeader header;
    char     block[1];
};

#define HEADER_SIZE (sizeof(MemoryBlock))
#define FIRST_BLOCK ((MemoryBlock*)MEMORY_START)

void    heap_init   (void){
    FIRST_BLOCK->header.used = 0;
    FIRST_BLOCK->header.size = (MEMORY_END - MEMORY_START) + HEADER_SIZE;
    FIRST_BLOCK->header.next = NULL;
}

void* heap_alloc(size_t size){
    char buff[32];
    MemoryBlock* block = FIRST_BLOCK;
    if (size % 4 != 0){
        size+=4 - (size % 4);   // always word aligned.
    }
    while(block && (block->header.used || block->header.size < size)){
        debug("block size ");
        debug(itoa(block->header.size,buff,10));
        debug("\n");
        block = block->header.next;
    }

    if (!block){
        // out of memory
        debug("No memory!\n");
        return NULL;
    }

    if (block->header.size + HEADER_SIZE > size){
        MemoryBlock* next = block->header.next;
        // split
        block->header.next = (MemoryBlock*) block + size + HEADER_SIZE;
        block->header.next->header.next = next;
    } 

    block->header.used = 1;
    block->header.size = size;

    debug("Allocated ");
    debug(itoa(size,buff,10));
    debug(" bytes at address ");
    debug(itoa((uint32_t)block->block,buff,16));
    debug("\n");

    return block->block;
}

void heap_free(void* address){
    MemoryBlock* block = (MemoryBlock*)(address - sizeof(BlockHeader));
    block->header.used = 0;
    // join free blocks
}
