#ifndef _ISR_H_
#define _ISR_H_
/**
 * Interrupt service routine handling
 **/
#include "board/cpu.h"

/**
 * ISR handler function.
 * The parameters recevied are the interrupt frame, and 
 * any user data passed to the install function
 **/
typedef void (*Isr)(InterruptFrame*, void*);

/**
 * Initializes the ISR handling routines
 **/
void    isr_init    (void);
/**
 * Installs an ISR handler, passing as parameters the number, the handler, and a 
 * user data which will be later passed to the handler
 **/
void    isr_install (uint16_t interrupt_number, Isr isr, void* callback_data);
/**
 * Installs an exception trap handler, passing as parameters the number, the handler, and a 
 * user data which will be later passed to the handler
 **/
void    trap_install(uint16_t interrupt_number, Isr isr, void* callback_data);
/**
 * Enables interrupts
 **/
//void    sti         (void);
#define sti() {   asm volatile("sti"); }
/**
 * Disables interrupts
 **/
//void    cli         (void);
#define cli() {   asm volatile("cli"); }
void    cld         (void);
/**
 * Just the start address of the ISR handler table
 * in assembly code
 **/
void    isr_handlers_start(void);

#endif
