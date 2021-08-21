#ifndef _STDDEF_H_
#define _STDDEF_H_

#ifndef NULL
#define NULL ((void*)0)
#endif

typedef unsigned int    size_t;
typedef int             ssize_t;
typedef unsigned int    ptrdiff_t;
typedef unsigned short  wchar_t;

#define offsetof(st,m)  ((ptrdiff_t)&((st*)NULL)->m)

#endif
