#ifndef _SHM_H_
#define _SHM_H_

typedef struct {
    void* memory;
} SharedMemory;

SharedMemory*   shared_memory_new       (uint32_t nblocks);
void            shared_memory_release   (SharedMemory* shared_memory);

#endif
