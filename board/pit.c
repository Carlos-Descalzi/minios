#include "board/pit.h"
#include "board/pic.h"
#include "board/io.h"
#include "lib/string.h"

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

static uint64_t ticks;

static void timer_handler(InterruptFrame* frame);

typedef struct {
    uint64_t ticks;
    uint64_t count;
    Isr callback;
} TimerSlot;

#define MAX_SLOTS   32

static TimerSlot slots[MAX_SLOTS];

void pit_init(){
    ticks = 0;
    memset(slots,0,sizeof(slots));
    pit_set_freq(1);// TODO: fix this
    isr_install(0x20, timer_handler); 
}

inline uint64_t pit_ticks(void){
    return ticks;
}

void pit_add_callback(Isr callback, uint64_t ticks){
    if (ticks > 0){
        for (int i=0;i<MAX_SLOTS;i++){
            if (!slots[i].callback){
                slots[i].callback = callback;
                slots[i].ticks = ticks;
                slots[i].count = 0;
                break;
            }
        }
    }
}

void pit_remove_callback(Isr callback){
    for (int i=0;i<MAX_SLOTS;i++){
        if (slots[i].callback = callback){
            slots[i].ticks = 0;
            slots[i].count = 0;
            slots[i].callback = NULL;
            break;
        }
    }
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
    uint32_t div = 119180 / freq; 

    outb(PORT_CMD, 0x36);
    outb(PORT_CH0, div & 0xFF);
    outb(PORT_CH0, div >> 8);
}

static void timer_handler(InterruptFrame* frame){
    cli();
    ticks++;
    for (int i=0;i<MAX_SLOTS;i++){
        if (slots[i].ticks && slots[i].callback){
            if (++slots[i].count == slots[i].ticks){
                slots[i].count = 0;
                slots[i].callback(frame);
            }
        }
    }

    pic_eoi1();
    sti();
}
