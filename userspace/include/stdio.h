#ifndef _STDIO_H_
#define _STDIO_H_
#include "stddef.h"

typedef struct _FILE FILE;

extern FILE*    stdin;
extern FILE*    stdout;
extern FILE*    stderr;

FILE*   fopen   (const char*pathname, const char* mode);
int     fclose  (FILE* stream);
size_t  fread   (void *buff, size_t size, size_t nmemb, FILE* stream);

#endif
