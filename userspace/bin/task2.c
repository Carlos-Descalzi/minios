#include "stdlib.h"
#include "sched.h"
#include "stdio.h"
/**
 * User space initialization program
 **/
int main(int argc,char **argv){
    while(1){
        printmessage("Task2 Looping\n");
        sched_yield();
    }

    return 0;
}
