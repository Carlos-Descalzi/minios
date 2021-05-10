#ifndef _STDIO_H_
#define _STDIO_H_

typedef struct __file FILE;

int kfprintf(FILE* fp, const char* format, ...);
int kfputc(int c, FILE* fp);

#endif
