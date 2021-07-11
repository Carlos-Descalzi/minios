#ifndef _STDLIB_H_
#define _STDLIB_H_

#include "stddef.h"

typedef void (*ListEnvFunc)(const char*);

void    exit    (int status) __attribute__ ((noreturn));
int     atoi    (const char*);
long    atol    (const char*);
char*   itoa    (int, char*,int);
char*   utoa    (unsigned int, char*,int);

void    listenv (ListEnvFunc);
char*   getenv  (const char* name);
int     putenv  (const char* env);
void*   malloc  (size_t size);
void    free    (void* ptr);

#endif
