#include "pit.h"
#include "io.h"

#define PORT_CH0            0x40
#define PORT_CH1            0x41
#define PORT_CH2            0x42
#define PORT_CMD            0x43

#define CMD_CH_0            0x00
#define CMD_CH_1            0x40
#define CMD_CH_2            0x80
#define CMD_READ_BACK       0xC0

#define ACC_LATCH_COUNT     0x00
#define ACC_ACC_LOB         0x01
#define ACC_ACC_HIGH        0x02
#define ACC_ACC_LOHI        0x03

#define MODE_INT_ON_TERM    0x00
#define MODE_ONE_SHOT       0x01
#define MODE_RATE_GEN       0x02
#define MODE_SQR_WAVE       0x03
#define MODE_SW_STROBE      0x04
#define MODE_HW_STROBE      0x05
#define MODE_RATE_GEN2      0x06
#define MODE_SQR_WAVE2      0x07


typedef struct {
    uint8_t
        bcd_binary: 1,
        operating_mode: 3,
        access_mode: 2,
        null_count: 1,
        output_state: 1;
} Status;

void pit_init(){
}

void pit_set_count(uint16_t count){
    outb(PORT_CH0, count & 0xFF);
    outb(PORT_CH0, ((count >> 8) & 0xFF));
}

uint16_t pit_get_count(){
    uint16_t count;

    outb(PORT_CMD,0x00);
    count = inb(PORT_CH0);
    count |= inb(PORT_CH0) << 8;

    return count;
}
void pit_set_freq(uint16_t freq){
    uint32_t div = 0xFFFF;//119180 / freq; TODO Fix

    outb(PORT_CMD, 0x36);
    outb(PORT_CH0, div & 0xFF);
    outb(PORT_CH0, div >> 8);
}
