#ifndef _TYPES_H_
#define _TYPES_H_

#include "stddef.h"

typedef unsigned char   u_char;
typedef unsigned short  u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;

typedef unsigned int    mode_t;
typedef unsigned int    pid_t;
typedef unsigned short  dev_t;
typedef unsigned long   off_t;
typedef unsigned int    uid_t;
typedef unsigned int    gid_t;
typedef unsigned long   time_t;
typedef unsigned long   suseconds_t;
typedef unsigned long   clock_t;

#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2

#endif
