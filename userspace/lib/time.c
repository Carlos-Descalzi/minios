#include <sys/time.h>
#include <string.h>
#include <syscall.h>

typedef struct {
    struct timeval tv;
    struct timezone tz;
} TimeData;

int gettimeofday(struct timeval *restrict tv, struct timezone *restrict tz){

    TimeData time_data;

    int result = syscall(SYS_GETTIME, &time_data);

    if (!result){
        memcpy(tv, &(time_data.tv),sizeof(struct timeval));
        memcpy(tz, &(time_data.tz),sizeof(struct timezone));
    }

    return result;
}
