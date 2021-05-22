#ifndef _PS2_H_
#define _PS2_H_

#include "lib/stdint.h"

#define PS2_IRQ 0x01

typedef struct {
    uint8_t controller_present:1,
            port1_present:1,
            port2_present:1,
            reserved:5;
} PS2Port;

void    ps2_get_status  (PS2Port* port);
uint8_t ps2_read        (uint8_t port);
void    ps2_write       (uint8_t port, uint8_t value);


#endif
