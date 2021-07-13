#ifndef _MMAN_H_
#define _MMAN_H_

#include "stddef.h"

void* mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);

#endif
