#include "board/ps2.h"
#include "board/io.h"



void    ps2_get_status  (PS2Port* port){
    uint8_t value;

    outb(0x64, CMD_TEST_CTRLR);
    value = inb(0x60);
    if (value == 0x55){
        port->controller_present = 1;
    }

    outb(0x64, CMD_TEST_1PORT);
    value = inb(0x60);
    if (value == 0x00){
        port->port1_present = 1;
    }

    outb(0x64, CMD_TEST_2PORT);
    value = inb(0x60);
    if (value == 0x00){
        port->port2_present = 1;
    }
}
uint8_t ps2_read        (uint8_t port){
    return inb(port);
}
void    ps2_write       (uint8_t port, uint8_t value){
    outb(port, value);
}
