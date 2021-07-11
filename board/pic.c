#include "board/pic.h"
#include "board/io.h"
#include "misc/debug.h"

#define PIC1            0x20
#define PIC2            0xa0
#define PIC1_CMD        PIC1
#define PIC1_DATA       (PIC1+1)
#define PIC2_CMD        PIC2
#define PIC2_DATA       (PIC2+1)

#define PIC1_IRQ_OFFSET PIC_IRQ_BASE
#define PIC2_IRQ_OFFSET (PIC_IRQ_BASE+0x8)

#define ICW1_ICW4	    0x01	
#define ICW1_SINGLE	    0x02	
#define ICW1_INTERVAL4	0x04
#define ICW1_LEVEL	    0x08
#define ICW1_INIT	    0x10
 
#define ICW4_8086	    0x01
#define ICW4_AUTO	    0x02
#define ICW4_BUF_SLAVE	0x08
#define ICW4_BUF_MASTER	0x0C
#define ICW4_SFNM	    0x10		

#define PIC_ISR         0x0b

#define io_wait()       // nothing for the moment

void pic_init(){
    /**
     * Remap PIC to avoid conflict with exceptions
     * source: https://wiki.osdev.org/PIC#Programming_with_the_8259_PIC
     **/

    uint8_t p1_mask, p2_mask;

    p1_mask = inb(PIC1_DATA);
    p2_mask = inb(PIC2_DATA);

    outb(PIC1_CMD, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4);
    io_wait();

    outb(PIC1_DATA, PIC1_IRQ_OFFSET);
    io_wait();
    outb(PIC2_DATA, PIC2_IRQ_OFFSET);
    io_wait();

    outb(PIC1_DATA, 4); // slave at irq2
    io_wait();
    outb(PIC2_DATA, 2); // cascade
    io_wait();

    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    //p2_mask =0;
    outb(PIC1_DATA, p1_mask);
    outb(PIC2_DATA, p2_mask);
    debug("PIC initialized\n");
}

void pic_enable(int pin){
    uint8_t mask;
    uint8_t port;
    if (pin < 8){
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        pin -= 8;
    }

    mask = inb(port);
    mask &= ~(1 << pin);
    outb(port, mask);
}
void pic_disable(int pin){
    uint8_t mask;
    uint8_t port;
    if (pin < 8){
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        pin -= 8;
    }

    mask = inb(port);
    mask |= 1 << pin;
    outb(port, mask);
}

inline void pic_eoi1(void){
    outb(0x20, 0x20);
}

inline void pic_eoi2(void){
    outb(0xA0, 0x20);
}

uint16_t pic_get_irq_reg(){
    outb(PIC1_CMD, PIC_ISR);
    outb(PIC2_CMD, PIC_ISR);
    uint8_t pic1 = inb(PIC1_CMD);
    uint8_t pic2 = inb(PIC2_CMD);
    return ((pic2 << 4) | pic1);
}

uint16_t pic_get_irq(){
    uint16_t reg = pic_get_irq_reg();
    uint16_t val = 0;
    if (reg & 0xF){
        reg &= 0xF;
        val=PIC1_IRQ_OFFSET-1;
    } else if (reg & 0xF0){
        reg >>=4;
        val=PIC2_IRQ_OFFSET-1;
    }
    while(reg){
        val++;
        reg >>=1;
    }
    return val;
}
