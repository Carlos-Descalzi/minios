#include "lib/heap.h"
#include "board/memory.h"
#include "lib/stdint.h"
#include "misc/debug.h"

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

#define HEADER_SIZE (sizeof(BlockHeader))
#define FIRST_BLOCK ((MemoryBlock*)HEAP_MEMORY_START_ADDRESS)

void heap_init(void){
    debug("HEAP - Total memory:"); 
    debug_i(HEAP_MEMORY_END_ADDRESS - HEAP_MEMORY_START_ADDRESS,10);
    debug("\n");
    FIRST_BLOCK->header.used = 0;
    FIRST_BLOCK->header.size = (HEAP_MEMORY_END_ADDRESS - HEAP_MEMORY_START_ADDRESS) + HEADER_SIZE;
    FIRST_BLOCK->header.next = NULL;
}

void* heap_alloc(size_t size){
    MemoryBlock* block = FIRST_BLOCK;
    if (size % 4 != 0){
        size+=4 - (size % 4);   // always word aligned.
    }
    while(block && (block->header.used || block->header.size < size)){
        //debug("HEAP - block size "); debug_i(block->header.size,10); debug("\n");
        block = block->header.next;
    }

    if (!block){
        // out of memory
        debug("HEAP - No memory!\n");
        return NULL;
    }

    if (block->header.size + HEADER_SIZE > size){
        MemoryBlock* next = block->header.next;
        // split
        block->header.size = size;
        block->header.next = (MemoryBlock*) (((char*)block) + size + HEADER_SIZE);
        block->header.next->header.size = block->header.size - size - HEADER_SIZE;
        block->header.next->header.next = next;
    } 

    block->header.used = 1;
    block->header.size = size;

    
    //debug("HEAP - Allocated "); debug_i(block->header.size,10); 
    //debug(" bytes at address "); debug_i((uint32_t)block->block,16); debug("\n");
    
    return block->block;
}

void heap_free(void* address){
    MemoryBlock* block = (MemoryBlock*)(address - HEADER_SIZE);
    block->header.used = 0;
    //debug("HEAP - Free "); debug_i(block->header.size + HEADER_SIZE,10); debug(" bytes");
    // TODO: join free blocks
    while(block->header.next && !block->header.next->header.used){
        block->header.size += block->header.next->header.size;
        block->header.next = block->header.next->header.next;
    }
    //debug(", collected ");debug_i(block->header.size + HEADER_SIZE,10); debug(" bytes\n");
}
