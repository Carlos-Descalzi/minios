#define NODEBUG
#include "lib/heap.h"
#include "board/memory.h"
#include "lib/stdint.h"
#include "misc/debug.h"

typedef struct MemoryBlock {
    uint32_t size:31,
             used:1;
    struct  MemoryBlock* next;
    char    block[];
} MemoryBlock;

#define HEADER_SIZE (sizeof(MemoryBlock))
#define HEAP_START  ((MemoryBlock*)HEAP_MEMORY_START_ADDRESS)
#define HEAP_END    ((MemoryBlock*)HEAP_MEMORY_END_ADDRESS)
#define HEAP_SIZE   (HEAP_MEMORY_END_ADDRESS - HEAP_MEMORY_START_ADDRESS)

void heap_init(void){
    debug("HEAP - Total memory:"); 
    debug_i(HEAP_SIZE,10);
    debug("\n");
    HEAP_START->used = 0;
    HEAP_START->size = HEAP_SIZE - HEADER_SIZE;
    HEAP_START->next = HEAP_END;
}

void* heap_alloc(size_t size){
    MemoryBlock* block = HEAP_START;
    if (size % 4 != 0){
        size+=4 - (size % 4);   // always word aligned.
    }
    while(block != HEAP_END
        && (block->used || block->size < ((uint32_t)size))){
        debug("HEAP - block size "); debug_i(block->size,10); debug(" - addr: ");debug_i(block,16);
        debug(" - req size: "); debug_i(size,10); debug(" - used: ");debug_i(block->used,10);
        debug(" - Next:");debug_i(block->next,16);
        debug("\n");
        block = block->next;
    }

    if (!block){
        // out of memory
        debug("HEAP - No memory!\n");
        return NULL;
    }
    if (block->used){
        debug("HEAP ALREADY USED !!!!\n");
        return NULL;
    }

    if (block->size - size > 2 * HEADER_SIZE){ 
        // I split the block only if the difference is 2 * header size
        // totally empirical
        uint32_t old_size = block->size;

        uint32_t total_block_size = size + HEADER_SIZE;

        MemoryBlock* next = block->next;

        block->size = size;
        block->next = ((void*)block) + total_block_size;

        block->next->size = old_size - total_block_size;
        block->next->next = next;
        block->next->used = 0;

        debug("HEAP - Block split :");debug_i(old_size,10);
        debug(",");debug_i(size,10);debug(",");
        debug_i(block->next->size,10);debug("\n");
    } 

    block->used = 1;
    
    debug("HEAP - Allocated "); debug_i(block->size,10); 
    debug(" bytes at address "); debug_i((uint32_t)block->block,16); debug("\n");
    
    return block->block;
}

void heap_free(void* address){
    MemoryBlock* block = (MemoryBlock*)(address - HEADER_SIZE);
    debug("HEAP - Free ");debug_i(address,16);debug("\n");
    block->used = 0;

    for (block = HEAP_START; block != HEAP_END; block = block->next){
        
        if (!block->used){
            // start to merge all unused consecutive blocks
            MemoryBlock* next_block = block->next;

            int size = 0;

            while (next_block != HEAP_END && !next_block->used){
                size += next_block->size + HEADER_SIZE;
                next_block = next_block->next;
            }
            block->size += size;
            block->next = next_block;

        }
    }

    int size = 0;

    MemoryBlock* free = HEAP_START;
    while(free != HEAP_END) {
        size += free->size;
        free = free->next;
    }
}
