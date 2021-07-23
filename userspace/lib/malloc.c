#include "stdlib.h"
#include "stdint.h"
#include "string.h"
/**
 * Very simple malloc based in double linked lists of blocks.
 * Each time a memory block is requested, a bigger block is split, or
 * if there is a block of the required size, is just returned.
 *
 * On free, it merges the released block with contiguous unused blocks.
 **/

typedef struct Block {
    uint32_t size:31,
             used:1;
    struct Block* prev;
    struct Block* next;
    char chunk[];
} Block;

#define HEADER_SIZE (sizeof(Block))
#define HEAP_SIZE   (HEAP_MEMORY_END_ADDRESS - HEAP_MEMORY_START_ADDRESS)

extern int _HEAP_START;
extern int _HEAP_END;

static Block* heap_start = NULL;

void* malloc(size_t size){

    if (!heap_start){
        // not initialized
        heap_start = (Block*) &_HEAP_START;
        heap_start->used = 0;
        heap_start->size = ((uint32_t)&_HEAP_END) - ((uint32_t)&_HEAP_START) - HEADER_SIZE;
        heap_start->prev = NULL;
        heap_start->next = NULL;
    }

    if (size % 4 != 0){
        size+=4 - (size % 4);   // always dword aligned.
    }
    for (Block* block = heap_start; block; block = block->next){
        if (!block->used){
            if (block->size - size > 2 * HEADER_SIZE){
                // split block if remainder is useful for another usable chunk
                Block* next = block->next;
                block->used = 1;
                block->next = (Block*) (block->chunk + size);
                block->next->prev = block;
                block->next->size = block->size - HEADER_SIZE - size;
                block->next->used = 0;
                block->next->next = next;
                block->size = size;
                return block->chunk;
            } else if (block->size >= size){
                // otherwise return the block even if is a bit bigger than requested.
                block->used = 1;
                return block->chunk;
            }
        }
    }
    return NULL;
}

void free(void* chunk){
    if (!chunk){
        return;
    }
    Block* block = (Block*) (chunk - HEADER_SIZE);
    block->used = 0;

    for (Block* b = block; b && !b->used; b = b->next){
        // merge contiguous unused blocks forward
        if (b->next && !b->next->used){
            b->size += b->next->size + HEADER_SIZE;
            b->next = b->next->next;
            if (b->next){
                b->next->prev = b;
            }
        }
    }
    for (Block* b = block; b && !b->used; b = b->prev){
        // merge contiguous unused blocks backward.
        if (b->prev && !b->prev->used){
            b->prev->size += b->size + HEADER_SIZE;
            b->prev->next = b->next;
            if (b->next){
                b->next->prev = b->prev;
            }
            b = b->prev;
        }
    }
}

void* calloc(size_t nmemb, size_t size){
    size_t total_size = nmemb * size;
    void* chunk = malloc(total_size);
    return memset(chunk, 0, total_size);
}
