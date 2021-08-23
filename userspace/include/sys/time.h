#ifndef _TIME_H_
#define _TIME_H_

#include <sys/types.h>

struct timeval {
   time_t      tv_sec;     /* seconds */
   suseconds_t tv_usec;    /* microseconds */
};

struct timezone {
   int tz_minuteswest;     /* minutes west of Greenwich */
   int tz_dsttime;         /* type of DST correction */
};

int gettimeofday(struct timeval *restrict tv, struct timezone *restrict tz);

#endif
