#ifndef _PS2_H_
#define _PS2_H_

#include "lib/stdint.h"

#define PS2_IRQ     0x01
#define PS2_IRQ2    0x12

#define PORT_DATA   0x60
#define PORT_STATUS 0x64
#define PORT_CMD    0x64

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
#define CMD_WRITE_PS2       0xD4

typedef struct {
    uint8_t controller_present:1,
            port1_present:1,
            port2_present:1,
            reserved:5;
} PS2Port;

void    ps2_get_status      (PS2Port* port);
uint8_t ps2_read            (uint8_t port);
uint8_t ps2_read_wait       (uint8_t port);
void    ps2_write           (uint8_t port, uint8_t value);

void    ps2_write_data      (uint8_t value);
void    ps2_write_data_ack  (uint8_t value);
uint8_t ps2_read_data       (void);

void    ps2_write_2         (uint8_t value);
void    ps2_write_2_ack     (uint8_t value);
#endif
