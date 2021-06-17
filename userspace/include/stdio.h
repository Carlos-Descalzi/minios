#ifndef _STDIO_H_
#define _STDIO_H_
#include "stddef.h"
#include "stdarg.h"

typedef struct _FILE FILE;

extern FILE*    stdin;
extern FILE*    stdout;
extern FILE*    stderr;

FILE*   fopen       (const char*pathname, const char* mode);
int     fclose      (FILE* stream);
size_t  fread       (void *buff, size_t size, size_t nmemb, FILE* stream);
int     fprintf     (FILE* fp, const char* format, ...);
int     vfprintf    (FILE* fp, const char* format, va_list ap);
int     fputc       (int c, FILE* fp);
int     printf      (const char* format, ...);
int     vprintf     (const char* format, va_list ap);
int     puts        (const char* str);

#endif
