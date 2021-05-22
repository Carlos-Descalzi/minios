#ifndef _ISR_H_
#define _ISR_H_
/**
 * Interrupt service routine handling
 **/

#include "lib/stdint.h"

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
} EFlags;

typedef struct {
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
    EFlags      flags;
    uint32_t    source_ss;
    uint32_t    source_esp;
} InterruptFrame;

typedef void (*Isr)(InterruptFrame);

void    isr_init    (void);
void    isr_install (uint16_t interrupt_number, Isr isr);
void    trap_install(uint16_t interrupt_number, Isr isr);
void    sti         (void);
void    cli         (void);
void    cld         (void);
/**
 * Just the start address of the ISR handler table
 * in assembly code
 **/
void    isr_handlers_start(void);

#endif
