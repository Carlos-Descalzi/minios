#ifndef _STDLIB_H_
#define _STDLIB_H_

#include "stddef.h"

typedef void (*ListEnvFunc)(const char*, void*);

void    exit    (int status) __attribute__ ((noreturn));
int     atoi    (const char*);
long    atol    (const char*);
char*   itoa    (int, char*,int);
char*   utoa    (unsigned int, char*,int);

void    listenv (ListEnvFunc, void* data);
char*   getenv  (const char* name);
int     putenv  (const char* env);
void*   malloc  (size_t size);
void    free    (void* ptr);
char*   realpath(const char* path, char* resolved_path);
void    copy_env(void* buffer, size_t buffer_size, int* env_count);

#endif
