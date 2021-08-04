#include "string.h"


void*   memcpy  (void* dest, const void* src, size_t n){

        for (int i=0;i<n;i++){
            ((char*)dest)[i] = ((char*)src)[i];
        }

    return dest;
}
void* memcpydw(void* dest, const void* src, size_t n){
    asm volatile(
            "\tmov %0, %%edi\n"
            "\tmov %1, %%esi\n"
            "\tmov %2, %%ecx\n"
            "\trep movsd\n"::"r"(dest),"r"(src),"r"(n)
    );


    return dest;
}
void* memset (void* s, int c, size_t size){
    asm volatile(
            "\tmov %0, %%edi\n"
            "\tmov %1, %%eax\n"
            "\tmov %2, %%ecx\n"
            "\trep stosb\n"::"r"(s),"r"(c),"r"(size)
    );
    return s;
}

void* memsetdw (void* s, uint32_t c, size_t dwords){
    asm volatile(
            "\tmov %0, %%edi\n"
            "\tmov %1, %%eax\n"
            "\tmov %2, %%ecx\n"
            "\trep stos\n"::"r"(s),"r"(c),"r"(dwords)
    );
    return s;
}

int memcmp(void* s1, void* s2, size_t n){

    for (int i=0;i<n;i++){
        int d = ((char*)s1)[i] - ((char*)s2)[i];

        if (d){
            return d;
        }
    }

    return 0;
}

char* strcpy(char* dest, const char* src){
    int i=0;
    do{
        dest[i] = src[i];
    } while(src[i++]);
    return dest;
}

char* strncpy(char* dest, const char* src, size_t n){
    if (dest && src && n){
        int i=0;
        do{
            dest[i] = src[i];
        } while(src[i++] && i < n);
    }
    return dest;
}

size_t strlen(const char* string){
    int len = 0;
    while(string[len]) len++;
    return len;
}
char*   strchr  (const char*s, int c){
    if (s){
        int i;
        for (i=0;s[i];i++){
            if (s[i] == c){
                return (char*) s+i;
            }
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

char* strstr(const char*str, const char* search){
    int l1 = strlen(str);
    int l2 = strlen(search);
    for (int i=0;i<l1;i++){
        if (!strncmp(str+i,search,l2)){
            return (char*) str+i;
        }
    }
    return NULL;

}

char* strtok_r (char* str, const char* delim, char** saveptr){
    char* token;
    if ((!str || !str[0]) && !*saveptr){
        return NULL;
    }
    if (*saveptr){
        token = *saveptr;
        *saveptr = strstr(*saveptr, delim); 
    } else {
        token = str;
        *saveptr = strstr(str, delim);
    }

    if (*saveptr){
        **saveptr = '\0';
        *saveptr += strlen(delim);
    }

    return token;
}
