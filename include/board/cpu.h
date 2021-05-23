#ifndef _CPU_H_
#define _CPU_H_

#include "lib/stdint.h"

/**
 * i386 CPU Registers and Flags
 **/

typedef union {
    uint32_t
        carry:1,
        reserved_1:1,
        parity:1,
        reserved_2:1,
        aux_carry:1,
        reserved_3:1,
        zero:1,
        sign:1,
        trap:1,
        interrupt_enable:1,
        direction:1,
        overflow:1,
        privilege_level:2,
        nested_task:1,
        reserved_4:1,
        resume:1,
        virtual_8086:1,
        reserved_5:14;
    uint32_t dwflags;
} X86EFlags;

typedef X86EFlags Flags;

typedef struct {
    uint32_t    cr3;
    uint32_t    edi;
    uint32_t    esi;
    uint32_t    ebp;
    uint32_t    esp;
    uint32_t    ebx;
    uint32_t    edx;
    uint32_t    ecx;
    uint32_t    eax;
    uint32_t    eip;
    uint32_t    cs;
    Flags       flags;
    uint32_t    source_ss;
    uint32_t    source_esp;
} X86CpuRegisters;

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
} X86TaskStateSegment;

typedef X86CpuRegisters CPUState;
typedef X86CpuRegisters InterruptFrame;
typedef X86TaskStateSegment TaskStateSegment;

#endif
