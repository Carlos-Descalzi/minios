#include "stdlib.h"
#include "sched.h"
#include "stdio.h"
/**
 * User space initialization program
 **/
int main(int argc,char **argv){


    while(1){
        printf("Running task 1\n");
        sched_yield();
    }

    return 0;
}
