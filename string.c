#include "string.h"

void* memcpy(void* dest, const void*src, size_t n){
    // TODO Remake in assembly for speed.
    int i;
    if (dest && src && n){
        for (i=0;i<n;i++){
            ((char*)dest)[i] = ((const char*)src)[i];
        }
    }
    return dest;
}
void* memset(void* s, int c, size_t size){
    int i;
    for (i=0;i<size;i++){
        ((char*)s)[i] = c;
    }
    return s;
}

char* strcpy(char* dest, const char* src){
    if (dest && src){
        int i=0;
        do{
            dest[i] = src[i];
        } while(src[i++]);
    }
    return dest;
}

size_t strlen(const char* string){
    int len = 0;
    if (string){
        while(string[len++]);
    }
    return len;
}
