#ifndef _ISR_H_
#define _ISR_H_
/**
 * Interrupt service routine handling
 **/
#include "board/cpu.h"

typedef void (*Isr)(InterruptFrame*, void*);

void    isr_init    (void);
void    isr_install (uint16_t interrupt_number, Isr isr, void* callback_data);
void    trap_install(uint16_t interrupt_number, Isr isr, void* callback_data);
void    sti         (void);
void    cli         (void);
void    cld         (void);
void    pausei      (void);
void    resumei     (void);
/**
 * Just the start address of the ISR handler table
 * in assembly code
 **/
void    isr_handlers_start(void);

#endif
