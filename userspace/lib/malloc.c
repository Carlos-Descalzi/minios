#include "stdlib.h"
#include "stdint.h"
/**
 * This is a very simple but unefficient malloc implementation,
 * will be replaced in the future
 **/

typedef struct MemoryBlock {
    uint32_t size:31,
             used:1;
    struct  MemoryBlock* next;
    char    block[];
} MemoryBlock;

#define HEADER_SIZE (sizeof(MemoryBlock))
#define HEAP_SIZE   (HEAP_MEMORY_END_ADDRESS - HEAP_MEMORY_START_ADDRESS)

extern int _HEAP_START;
extern int _HEAP_END;

static MemoryBlock* heap_start = NULL;
static MemoryBlock* heap_end = NULL;

void* malloc(size_t size){

    if (!heap_start){
        // not initialized
        heap_start = (MemoryBlock*) &_HEAP_START;
        heap_end = (MemoryBlock*) &_HEAP_END;
        heap_start->used = 0;
        heap_start->size = ((uint32_t)heap_end) - ((uint32_t)heap_start) - HEADER_SIZE;
        heap_start->next = heap_end;
    }

    MemoryBlock* block = heap_start;
    if (size % 4 != 0){
        size+=4 - (size % 4);   // always word aligned.
    }
    while(block != heap_end
        && (block->used || block->size < ((uint32_t)size))){
        block = block->next;
    }

    if (!block){
        // out of memory
        return NULL;
    }
    if (block->used){
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
    } 

    block->used = 1;
    
    return block->block;
}

void free(void* address){
    MemoryBlock* block = (MemoryBlock*)(address - HEADER_SIZE);
    block->used = 0;

    for (block = heap_start; block != heap_end; block = block->next){
        
        if (!block->used){
            // start to merge all unused consecutive blocks
            MemoryBlock* next_block = block->next;

            int size = 0;

            while (next_block != heap_end && !next_block->used){
                size += next_block->size + HEADER_SIZE;
                next_block = next_block->next;
            }
            block->size += size;
            block->next = next_block;

        }
    }

    int size = 0;

    MemoryBlock* free = heap_start;
    while(free != heap_end) {
        size += free->size;
        free = free->next;
    }
}
