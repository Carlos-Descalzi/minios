#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include "lib/stdint.h"

typedef struct {
    uint16_t link;
    uint16_t reserved_1;
    uint32_t esp0;
    uint16_t ss0;
    uint16_t reserved_2;
    uint32_t esp1;
    uint16_t ss1;
    uint16_t reserved_3;
    uint32_t esp2;
    uint16_t ss2;
    uint16_t reserved_4;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint16_t es;
    uint16_t reserved_5;
    uint16_t cs;
    uint16_t reserved_6;
    uint16_t ss;
    uint16_t reserved_7;
    uint16_t ds;
    uint16_t reserved_8;
    uint16_t fs;
    uint16_t reserved_9;
    uint16_t gs;
    uint16_t reserved_10;
    uint16_t ldtr;
    uint32_t reserved_11;
    uint16_t iopb_offset;
} TaskStateSegment;

extern const TaskStateSegment* tss;

#define KERNEL_TSS  0x18


#endif
