#define NODEBUG
#include "board/pit.h"
#include "board/pic.h"
#include "board/io.h"
#include "lib/string.h"
#include "misc/debug.h"

#define PIT_IRQ             0x00

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

static Isr timer_handler = NULL;
void* timer_handler_data = NULL;

static void pit_handle_irq(InterruptFrame* frame, void* data);

void pit_init(){
    pit_set_freq(1);// TODO: fix this
    //isr_install(0x20, timer_handler, NULL); 
    isr_install(PIC_IRQ_BASE + PIT_IRQ, pit_handle_irq, NULL); 
    debug("PIT Initialized\n");
}

void pit_set_isr_handler(Isr isr, void* data){
    timer_handler = isr;
    timer_handler_data = data;
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
    /*
    */
    /*outb(PORT_CMD, 0x31);
    outb(PORT_CH0,  1);
    outb(PORT_CH0,  0);*/
}
static void pit_handle_irq(InterruptFrame* frame, void* data){
    cli();
    pic_eoi1();
    sti();
    if (timer_handler){
        timer_handler(frame, timer_handler_data);
    }
}
