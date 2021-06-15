#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#include "lib/stdint.h"

typedef uint32_t Lock;

void    acquire_lock(Lock *lock);
void    release_lock(Lock* lock);


#endif
