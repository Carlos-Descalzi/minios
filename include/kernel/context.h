#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include "lib/stdint.h"
#include "board/cpu.h"

extern const TaskStateSegment* tss;

#define KERNEL_TSS  0x18


#endif
