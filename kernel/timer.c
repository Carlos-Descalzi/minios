#include <kernel/timer.h>
#include <lib/string.h>
#include <kernel/isr.h>
#include <board/pic.h>
#include <board/pit.h>
#include <misc/debug.h>

typedef struct {
    uint64_t usecs;
    uint64_t count;
    uint64_t total_count;
    TimerCallback callback;
    void* callback_data;
} TimerSlot;

#define MAX_SLOTS   32

static uint64_t ticks;
static TimerSlot slots[MAX_SLOTS];
static uint64_t usecs_per_tick;

static void timer_handler(InterruptFrame* frame, void* data);

void timer_init(){
    ticks = 0;
    usecs_per_tick = 30000000;
    memset(slots, 0, sizeof(slots));
    pit_set_isr_handler(timer_handler, NULL);
}

void timer_add(uint64_t usecs, TimerCallback callback, void* user_data){
    if (usecs > 0){
        for (int i=0;i<MAX_SLOTS;i++){
            if (!slots[i].callback){
                slots[i].callback = callback;
                slots[i].callback_data = user_data;
                slots[i].usecs = usecs;
                slots[i].total_count = 0;
                slots[i].count = 0;
                break;
            }
        }
    }
}

void timer_remove(TimerCallback callback){
    for (int i=0;i<MAX_SLOTS;i++){
        if (slots[i].callback == callback){
            slots[i].usecs = 0;
            slots[i].count = 0;
            slots[i].total_count = 0;
            slots[i].callback = NULL;
            slots[i].callback_data = NULL;
            break;
        }
    }
}

static void timer_handler(InterruptFrame* frame, void* data){
    ticks++;
    for (int i=0;i<MAX_SLOTS;i++){
        if (slots[i].callback){
            slots[i].count += usecs_per_tick;
            if (slots[i].count >= slots[i].usecs){
                slots[i].total_count += slots[i].count;
                slots[i].count = 0;
                slots[i].callback(
                    slots[i].total_count,
                    slots[i].callback_data
                );
            }
        }
    }

}
