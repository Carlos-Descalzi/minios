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

uint8_t ps2_read (uint8_t port){
    //while(!(inb(0x64) & 1))  { asm ("pause"); }
    //wait_in();
    return inb(port);
}

void ps2_write (uint8_t port, uint8_t value){
//    while(inb(0x64) & 2) { asm ("pause"); }
    outb(port, value);
}

void ps2_write_2_ack (uint8_t value){
    ps2_write(PORT_CMD, CMD_WRITE_PS2);
    ps2_write(PORT_DATA, value);
    while (ps2_read(PORT_DATA) != 0xFA);
}

void ps2_write_2 (uint8_t value){
    ps2_write(PORT_CMD, CMD_WRITE_PS2);
    ps2_write(PORT_DATA, value);
}

uint8_t ps2_read_data (void){
    ps2_write(PORT_CMD, CMD_READ_BYTE);
    return ps2_read(PORT_DATA);
}

void ps2_write_data(uint8_t value){
    ps2_write(PORT_CMD, CMD_WRITE_BYTE);
    ps2_write(PORT_DATA, value);
}
