#ifndef _IO_H_
#define _IO_H_
/**
 * Basic IO Operations
 **/

#include "lib/stdint.h"

void outb (uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);

void outw (uint16_t port, uint16_t value);
uint16_t inw(uint16_t port);

void outdw(uint16_t port, uint32_t value);
uint32_t indw(uint16_t port);


#endif
