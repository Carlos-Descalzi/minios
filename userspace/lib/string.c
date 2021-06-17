#include "string.h"


void*   memcpy  (void* dest, const void* src, size_t n){

    if (dest && src){
        for (int i=0;i<n;i++){
            ((char*)dest)[i] = ((char*)src)[i];
        }
    }

    return dest;
}
void* memset  (void* s, int c, size_t size){
    for (int i=0;i<size;i++){
        ((char*)s)[i] = c;
    }
    return s;
}


char* strcpy(char* dest, const char* src){
    int i=0;
    do{
        dest[i] = src[i];
    } while(src[i++]);
    return dest;
}

size_t strlen(const char* string){
    int len = 0;
    while(string[len]) len++;
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
int strcmp(const char*s1, const char* s2){
    int i;
    for (i=0;s1[i] && s2[i] && (s1[i] == s2[i]);i++);
    return s1[i] - s2[i];
}
int strncmp(const char*s1, const char* s2, size_t n){
    int i;
    int d=0;
    for (i=0;i<n && d==0;i++){
        d = s1[i] - s2[i];
    }
    return d;
}
