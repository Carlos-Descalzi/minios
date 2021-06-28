#ifndef _STRING_H_
#define _STRING_H_

#include "stddef.h"

void*   memcpy  (void* dest, const void* src, size_t n);
void*   memset  (void* s, int c, size_t size);
char*   strcpy  (char* dest, const char* src);
size_t  strlen  (const char* string);
char*   strchr  (const char*s, int c);
char*   strrchr (const char*s, int c);
int     strcmp  (const char*s1, const char* s2);
int     strncmp (const char*s1, const char* s2, size_t n);
char*   strcat  (char* dest, const char* src);

#endif
