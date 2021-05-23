#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "board/cpu.h"
#include "kernel/paging.h"

typedef struct {
    uint32_t            cpu_ticks;
    CPUState            cpu_state;
    PageDirectoryEntry* page_directory;
} Process;

#endif
