#ifndef _SHM_H_
#define _SHM_H_

#include "lib/stdint.h"

typedef struct {
    uint32_t n_pages;
    uint32_t page_addresses[];
} SharedMemory;

SharedMemory*   shared_memory_new       (uint32_t n_pages);
void            shared_memory_release   (SharedMemory* shared_memory);
int             shared_memory_count     (void);
SharedMemory*   shared_memory_nth       (int index);

#endif
