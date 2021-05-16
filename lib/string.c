#include "lib/string.h"

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
char*   strchr  (const char*s, int c){
    int i;
    for (i=0;s[i];i++){
        if (s[i] == c){
            return (char*)&(s[i]);
        }
    }
    return NULL;
}
char*   strrchr (const char*s, int c){
    int i;
    for (i=strlen(s);i>=0;i--){
        if (s[i] == c){
            return (char*)&(s[i]);
        }
    }
    return NULL;
}
