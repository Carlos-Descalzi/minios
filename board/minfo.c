#include "board/minfo.h"
#include "misc/debug.h"
#include "lib/stdlib.h"

#define MACHINE_INFO_BASE   0x8000

typedef struct __attribute__((__packed__)) {
    uint32_t entry_count;
    MemoryRegion regions[];
} MemoryInfo;

void minfo_iterate_regions(MemRegionVisitor visitor, void *user_data){
    uint8_t i;
    MemoryInfo* info = (MemoryInfo*)MACHINE_INFO_BASE;
    MemoryRegion* regions = info->regions;

    for (i=0;i<info->entry_count;i++){
        if (visitor(&(regions[i]), i, user_data)){
            break;
        }
    }
}
