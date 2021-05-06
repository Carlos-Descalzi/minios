#ifndef _STRING_H_
#define _STRING_H_

#include "stddef.h"

void*   memcpy  (void* dest, const void* src, size_t n);
char*   strcpy  (char* dest, const char* src);
size_t  strlen  (const char* string);

#endif
