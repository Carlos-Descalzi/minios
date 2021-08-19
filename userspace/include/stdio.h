#ifndef _STDIO_H_
#define _STDIO_H_
#include "stddef.h"
#include "stdarg.h"

#define EOF (-1)

#define BUFSIZ      1024
#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2

#define _IONBF      0
#define _IOLBF      1
#define _IOFBF      2

typedef struct _FILE FILE;

extern FILE*    stdin;
extern FILE*    stdout;
extern FILE*    stderr;

FILE*   fopen       (const char*pathname, const char* mode);
int     fclose      (FILE* stream);
size_t  fread       (void *buff, size_t size, size_t nmemb, FILE* stream);
int     fprintf     (FILE* fp, const char* format, ...);
int     vfprintf    (FILE* fp, const char* format, va_list ap);
int     printf      (const char* format, ...);
int     vprintf     (const char* format, va_list ap);
int     sprintf     (char* str, const char* format, ...);
int     vsprintf    (char* str, const char* format, va_list ap);
char*   fgets       (char* buff, int size, FILE* stream);
int     puts        (const char* str);
int     fputs       (const char* str, FILE* stream);
int     fputc       (int c, FILE* fp);
int     putchar     (int c);
size_t  fwrite      (const void* ptr, size_t size, size_t nmemb, FILE* stream);
int     fseek       (FILE* fp, long offset, int whence);
long    ftell       (FILE* fp);
int     ungetc      (int c, FILE* fp);
FILE*   tmpfile     (void);
int     fgetc       (FILE*);
int     setvbuf     (FILE*, char*, int, size_t);
int     remove      (const char*);
int     rename      (const char*, const char*);
char    *tmpnam     (char *s);


#define getc(f)     fgetc(f)

#endif
