#ifndef _STDDEF_H_
#define _STDDEF_H_

#ifndef NULL
#define NULL ((void*)0)
#endif

typedef unsigned int    size_t;
typedef int             ssize_t;

typedef unsigned short  dev_t;
typedef unsigned int    pid_t;

typedef unsigned int    off_t;

typedef unsigned long   time_t;
typedef unsigned long   suseconds_t;

#define offsetof(st,m)  ((unsigned int)&((st*)NULL)->m)
#endif
