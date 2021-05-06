#ifndef _ISR_H_
#define _ISR_H_
/**
 * Interrupt service routine handling
 **/

#include "stdint.h"

#define INTERRUPT __attribute__((interrupt))

typedef struct {
    uint32_t eip;
    uint32_t cs;
    uint32_t flags;
    uint32_t esp;
    uint32_t ss;
} InterruptFrame;

typedef void (*Isr)(InterruptFrame*);

void    isr_install (uint16_t interrupt_number, Isr isr);
void    sti         (void);
void    cli         (void);
void    cld         (void);

#endif
