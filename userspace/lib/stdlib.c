#include "stdlib.h"
#include "syscall.h"
#include "path.h"

const char NUMBERS[] = "0123456789ABCDEF";

typedef struct {
    int nenv;
    const char* vars[];
} Env;

const extern Env env_ptr;

#define ATON(string,value) { \
    int l;\
    for (l=0;string[l];l++){ \
        if (string[l] >='0' && string[l] <='9'){\
            value*=10;\
            value+=(int)string[l] - '0';\
        } else if (string[l] == '-'){\
            value=-value;\
        }\
    }\
}

int atoi(const char* string){
    int value = 0;
    ATON(string,value);
    return value;
}
long atol(const char* string){
    long value = 0;
    ATON(string,value);
    return value;
}
char* itoa(int value, char* string,int radix){
    int d, i, m;
    i=0;
    string[0] = '\0';
    do {
        d = value % radix;
        string[i++]=NUMBERS[d];
        value /= radix;
    } while(value > 0);
    if (value < 0){
        string[i++] = '-';
    }
    string[i] = '\0';

    if (i == 2){
        d = string[1];
        string[1] = string[0];
        string[0] = d;
    } else {
        m = i/2;
        i--;
        for(;m>=0;m--){
            d = string[m];
            string[m] = string[i-m];
            string[i-m] = d;
        }
    }
        
    return string;
}

char* utoa(unsigned int value, char* string,int radix){
    int d, i, m;
    i=0;
    string[0] = '\0';
    do {
        d = value % radix;
        string[i++]=NUMBERS[d];
        value /= radix;
    } while(value > 0);
    string[i] = '\0';

    if (i == 2){
        d = string[1];
        string[1] = string[0];
        string[0] = d;
    } else {
        m = i/2;
        m--;
        i--;
        for(;m>=0;m--){
            d = string[m];
            string[m] = string[i-m];
            string[i-m] = d;
        }
    }
        
    return string;
}
 
void exit(int status){
    syscall(SYS_EXIT, (void*)status);
    while(1);
}

char* realpath(const char* path, char* resolved_path){
    return path_absolute(path, resolved_path);
}
