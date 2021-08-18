#ifndef _STDLIB_H_
#define _STDLIB_H_

#include "stddef.h"

#define min(a,b) (a<b ? a : b)
#define max(a,b) (a>b ? a : b)
#define abs(a)   (a>=0 ? a : -a)

typedef void (*ListEnvFunc)(const char*, void*);

void    exit        (int status) __attribute__ ((noreturn));
int     atoi        (const char*);
long    atol        (const char*);
char*   itoa        (int, char*,int);
char*   utoa        (unsigned int, char*,int);

void    listenv     (ListEnvFunc, void* data);
char*   getenv      (const char* name);
int     putenv      (const char* env);
void    copy_env    (void* buffer, size_t buffer_size, int* env_count);

char*   realpath    (const char* path, char* resolved_path);

void*   malloc      (size_t size);
void*   calloc      (size_t nmemb, size_t size);
void    free        (void* ptr);

#endif
