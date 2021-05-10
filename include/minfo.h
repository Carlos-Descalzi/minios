#ifndef _MINFO_H_
#define _MINFO_H_
/**
 * Machine information
 **/
#include "stdint.h"

#define MEM_TYPE_FREE       1
#define MEM_TYPE_RESERVED   2

typedef struct __attribute__((__packed__)) {
    uint64_t base_address;
    uint64_t length;
    uint32_t type;
    uint32_t acpi;
} MemoryRegion;

typedef uint8_t (*MemRegionVisitor)(MemoryRegion*, uint8_t, void*);

void    minfo_iterate_regions(MemRegionVisitor visitor, void* user_data);

#endif
