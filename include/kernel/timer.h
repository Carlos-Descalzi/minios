#ifndef _TIMER_H_
#define _TIMER_H_

#include <lib/stdint.h>

typedef void (*TimerCallback)(uint64_t, void*);

void timer_init     (void);
void timer_add      (uint64_t usecs, TimerCallback callback, void* user_data);
void timer_remove   (TimerCallback callback);

#endif
