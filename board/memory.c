#include "board/memory.h"
#include "board/minfo.h"
#include "lib/string.h"
#include "misc/debug.h"

#define USER_MEMORY_BITMAP  ((uint32_t*)USER_MEMORY_BITMAP_ADDRESS)

static uint32_t frame_bitmap_length;

static uint8_t check_avail_ram(MemoryRegion* region, uint8_t number, void* data);

void memory_init(void){
    uint32_t available_ram;
    minfo_iterate_regions(check_avail_ram, &available_ram);
    frame_bitmap_length = available_ram >> 15;
    memset(USER_MEMORY_BITMAP,0,frame_bitmap_length);
    debug("Available User RAM:");debug_i(frame_bitmap_length << 15,10);debug(" Bytes.\n");
    debug("Frame bitmap size:");debug_i(frame_bitmap_length * 4,10);debug(" Bytes.\n");
}

uint32_t memory_alloc_block(void){
    int i, j;
    for (i=0;i<frame_bitmap_length;i++){
        for (j=0;j<32;j++){
            if ((USER_MEMORY_BITMAP[i] & 1 << j) == 0){
                USER_MEMORY_BITMAP[i] |= 1 << j;
                return USER_MEMORY_START_ADDRESS + (i * 32 + j) * MEMORY_BLOCK_SIZE;
            }
        }
    }
    return 0;
}

void memory_free_block(uint32_t block_address){
    uint32_t bit;
    uint32_t index;
    block_address -= USER_MEMORY_START_ADDRESS;
    block_address /= MEMORY_BLOCK_SIZE;
    bit = 1 << (block_address % 32);
    index = block_address / 32;
    USER_MEMORY_BITMAP[index] &= ~bit; 
}

static uint8_t check_avail_ram(MemoryRegion* region, uint8_t number, void* data){
    if (number == 3){
        *((uint32_t*)data) = region->length;
        return 1;
    }
    return 0;
}
