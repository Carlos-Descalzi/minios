#include <board/rtc.h>

#include <board/pic.h>

#define RTC_IRQ         0x08
#define PORT_RTC_REG    0x70
#define PORT_RTC_DATA   0x71

#define RTC_SECONDS     0x00
#define RTC_MINUTES     0x02
#define RTC_HOURS       0x04
#define RTC_WEEK_DAY    0x06
#define RTC_MONTH_DAY   0x07
#define RTC_MONTH       0x08
#define RTC_YEAR        0x09
#define RTC_CENTURY     0x32

#define RTC_STATUS_A    0x0A
#define RTC_STATUS_B    0x0B

static uint8_t  read_reg    (uint8_t reg);
static void     write_reg   (uint8_t reg, uint8_t value);

void rtc_init(){
    // set rate
    write_reg(
        RTC_STATUS_A, 
        read_reg(RTC_STATUS_A) | 0x03
    );
    // enable interrupt
    write_reg(
        RTC_STATUS_B,
        read_reg(RTC_STATUS_B) | 0x40
    );
}

int rtc_get_time(RtcTime* time){
    time->seconds = read_reg(RTC_SECONDS);
    time->minutes = read_reg(RTC_MINUTES);
    time->hours = read_reg(RTC_HOURS);
    time->week_day = read_reg(RTC_WEEK_DAY);
    time->month_day = read_reg(RTC_MONTH_DAY);
    time->month = read_reg(RTC_MONTH);
    time->year = read_reg(RTC_YEAR);
    time->century = read_reg(RTC_CENTURY);

    return 0;
}

void rtc_set_isr_handler(Isr isr, void* data){
    isr_install(PIC_IRQ_BASE + RTC_IRQ, isr, NULL); 
}

static uint8_t read_reg(uint8_t reg){
    outb(PORT_RTC_REG, reg);
    return inb(PORT_RTC_DATA);
}

static void write_reg(uint8_t reg, uint8_t value){
    outb(PORT_RTC_REG, reg);
    outb(PORT_RTC_DATA, value);
}
