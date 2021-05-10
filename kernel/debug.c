#include "debug.h"
#include "io.h"
#include "stdlib.h"


void serial_log(const char* string){
    int i;

    for (i=0;string[i];i++){
        outb(0x3F8,string[i]);
    }
}

void serial_log_i(long long number, unsigned char radix){
    char buff[20];
    serial_log(itoa(number,buff,radix));
}

void serial_log_c(char c){
    outb(0x3F8,c);
}
