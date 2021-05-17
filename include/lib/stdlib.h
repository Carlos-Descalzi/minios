#ifndef _STDLIB_H_
#define _STDLIB_H_

#define min(a,b) (a<b ? a : b)
#define max(a,b) (a>b ? a : b)

int atoi(const char*);
long atol(const char*);

char* itoa(int, char*,int);
char* utoa(unsigned int, char*,int);
#endif
