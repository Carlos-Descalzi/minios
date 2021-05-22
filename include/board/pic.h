#ifndef _PIC_H_
#define _PIC_H_

#include "lib/stdint.h"

/**
 * PIC is remapped to start on 0x20
 **/
#define     PIC_IRQ_BASE    0x020
/**
 * Programmable Interrupt Controller
 **/
void        pic_init        (void);
/*
 * Acknowledges interrupt on PIC.
 **/
void        pic_eoi         (void);
/**
 * Returns both PIC1 (LSB) and PIC2 (MSB) in a 16 bit integer
 **/
uint16_t    pic_get_irq_reg (void);
/**
 * Returns the ISR number
 **/
uint16_t    pic_get_irq     (void);
#endif
