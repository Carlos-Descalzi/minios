#ifndef _PIT_H_
#define _PIT_H_
/**
 * Programmable Interval Timer
 **/

#include "lib/stdint.h"
#include "kernel/isr.h"

void        pit_init            (void);
void        pit_set_count       (uint16_t count);
uint16_t    pit_get_count       (void);
void        pit_set_freq        (uint16_t freq);
uint64_t    pit_ticks           (void);
void        pit_set_isr_handler (Isr isr, void* data);

#endif
