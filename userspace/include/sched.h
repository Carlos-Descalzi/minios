#ifndef _SCHED_H_
#define _SCHED_H_

#define COND_TYPE_FD    1
#define COND_TYPE_MSG   2

typedef struct {
    int cond_type;
    int fd;
} WaitCondition;


int sched_yield             (void);
int sched_wait_conditions   (int n_conditions, WaitCondition* conditions);


#endif
