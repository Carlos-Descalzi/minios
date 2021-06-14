#ifndef _STDLIB_H_
#define _STDLIB_H_

void exit(int status) __attribute__ ((noreturn));
void printmessage(const char* message);
int atoi(const char*);
long atol(const char*);

char* itoa(int, char*,int);
char* utoa(unsigned int, char*,int);

#endif
