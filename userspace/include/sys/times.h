#ifndef _TIMES_H_
#define _TIMES_H_

#include "stddef.h"
#include "stdint.h"

struct tms {
    uint32_t tms_cutime;
    uint32_t tms_stime;
    uint32_t tms_cstime;
    uint32_t tms_utime;
};

clock_t times(struct tms *);

#endif
