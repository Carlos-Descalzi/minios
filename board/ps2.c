#include "board/ps2.h"
#include "board/io.h"

#define CMD_READ_BYTE       0x20
#define CMD_WRITE_BYTE      0x60
#define CMD_DISABLE_2PORT   0xA7
#define CMD_ENABLE_2PORT    0xA8
#define CMD_TEST_2PORT      0xA9
#define CMD_TEST_CTRLR      0xAA
#define CMD_TEST_1PORT      0xAB
#define CMD_DIAGNOSIS       0xAC
#define CMD_DISABLE_1PORT   0xAD
#define CMD_ENABLE_1PORT    0xAE
#define CMD_READ_INPUT      0xC0
#define CMD_LSB_BITS        0xC1
#define CMD_MSG_BITS        0xC2
#define CMD_READ_OUTPUT     0xD0

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
    return inb(0x60);
}
void    ps2_write       (uint8_t port, uint8_t value){
}
