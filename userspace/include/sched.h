#ifndef _SCHED_H_
#define _SCHED_H_

#define COND_TYPE_FD    1
#define COND_TYPE_MSG   2

typedef struct {
    int cond_type;
    int fd;
} WaitCondition;

/**
 * Moves the current process out of execution, leaving the place
 * for another process
 **/
int sched_yield             (void);
/**
 * Puts process into wait state, until any of a list of conditions applies.
 * Conditions can be, data available in a file descriptor, or an incoming message available
 **/
int sched_wait_conditions   (int n_conditions, WaitCondition* conditions);


#endif
