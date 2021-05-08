#include "debug.h"
#include "io.h"


void serial_log(const char* string){
    int i;

    for (i=0;string[i];i++){
        outb(0x3F8,string[i]);
    }
}
