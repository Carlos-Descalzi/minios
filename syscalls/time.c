#include <kernel/syscalls.h>
#include <kernel/task.h>
#include <board/rtc.h>

struct timeval {
   uint64_t    tv_sec;     
   uint64_t    tv_usec;   
};

struct timezone {
   uint32_t    tz_minuteswest;   
   uint32_t    tz_dsttime;      
};

typedef struct {
    struct timeval tv;
    struct timezone tz;
} TimeData;

uint32_t syscall_gettime (SyscallArg arg){

    TimeData* time_data = tasks_to_kernel_address(arg.ptr_arg, sizeof(TimeData));

    RtcTime time;

    rtc_get_time(&time);

    uint64_t time_to_sec = 0;
    

    time_data->tv.tv_usec = 0;          // TODO
    time_data->tz.tz_dsttime = 0;       // TODO
    time_data->tz.tz_minuteswest = 0;   // TODO
    time_data->tv.tv_sec = time_to_sec;

    return 0;
}
