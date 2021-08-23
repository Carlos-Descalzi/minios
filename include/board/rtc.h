#ifndef _RTC_H_
#define _RTC_H_

#include <lib/stdint.h>
#include <kernel/isr.h>

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t week_day;
    uint8_t month_day;
    uint8_t month;
    uint8_t year;
    uint8_t century;
} RtcTime;

void    rtc_init            (void);
int     rtc_get_time        (RtcTime* time);
void    rtc_set_isr_handler (Isr isr, void* data);

#endif
