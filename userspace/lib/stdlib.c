#include "stdlib.h"
#include "syscall.h"

void exit(int status){
    syscall(SYS_EXIT, (void*)status);
    while(1);
}
