#include <unistd.h>
#include <syscall.h>

typedef struct {
    uint32_t seconds;
    uint64_t nsecs;
} SleepData;

unsigned int sleep (unsigned int seconds){
    SleepData sleep_data = {
        .seconds = seconds,
        .nsecs = 0
    };
    syscall(SYS_SLEEP, &sleep_data);

    return 0;
}
unsigned int usleep (unsigned int milliseconds){
    SleepData sleep_data = {
        .seconds = 0,
        .nsecs = milliseconds * 1000000
    };
    syscall(SYS_SLEEP, &sleep_data);

    return 0;
}
