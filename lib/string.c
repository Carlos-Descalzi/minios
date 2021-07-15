#include "lib/string.h"

char* strcpy(char* dest, const char* src){
    int i=0;
    do{
        dest[i] = src[i];
    } while(src[i++]);
    return dest;
}

char* strncpy(char* dest, const char* src, size_t count){
    int i;

    for (i=0;i<count;i++){
        dest[i] = src[i];
        if(!src[i]){
            break;
        }
    }
    dest[i] = '\0';

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
char* strcat(char* dest, const char* src){
    int i;
    for (i=0;dest[i];i++);
    strcpy(dest+i,src);
    return dest;
}
int memcmp(const void* m1, const void* m2, size_t s){
    int r = 0;
    for (int i=0;i< s;i++){
        r = ((char*)m1) - ((char*)m2);
        if (r){
            return r;
        }
    }
    return r;
}
