#ifndef _HEAP_H_
#define _HEAP_H_

#include "stdint.h"

void        heap_init       (void);
void*       heap_alloc      (size_t size);
void        heap_free       (void* block);
void        heap_stats      (uint32_t* total, uint32_t* free);

#define heap_new(t) heap_alloc(sizeof(t))

#endif
