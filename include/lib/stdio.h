#ifndef _STDIO_H_
#define _STDIO_H_

typedef struct __file FILE;

int fprintf(FILE* fp, const char* format, ...);
int fputc(int c, FILE* fp);

#endif
