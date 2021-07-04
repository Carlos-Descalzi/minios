#ifndef _STDLIB_H_
#define _STDLIB_H_

typedef void (*ListEnvFunc)(const char*);

void exit(int status) __attribute__ ((noreturn));
int atoi(const char*);
long atol(const char*);

char* itoa(int, char*,int);
char* utoa(unsigned int, char*,int);
void  listenv(ListEnvFunc);
char* getenv(const char* name);

#endif
