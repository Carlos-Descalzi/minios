#ifndef _HEAP_H_
#define _HEAP_H_

#include "stddef.h"

void    heap_init   (void);
void*   heap_alloc  (size_t size);
void    heap_free   (void* block);

#define heap_new(t) heap_alloc(sizeof(t))

#endif
