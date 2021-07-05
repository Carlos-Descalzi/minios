#include "stdlib.h"
#include "sched.h"
#include "stdio.h"
#include "modules.h"
#include "spawn.h"
/**
 * User space initialization program, not yet started nor in use.
 **/
int main(int argc,char **argv){
    char buff[32];
    FILE* fp;

    printf("Loading modules ...");

    fp = fopen("disk0:/etc/modules.conf","r");

    while(fgets(buff,32,fp)){
        printf("\tLoading %s\n",buff);
        modules_load(buff);
    }

    spawn("disk0:/shell.elf",0,NULL,0,NULL);

    fclose(fp);

    return 0;
}
