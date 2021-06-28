#include "stdlib.h"
#include "sched.h"
#include "stdio.h"
#include "modules.h"
#include "spawn.h"
/**
 * User space initialization program
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

    spawn("disk0:/shell.elf");
    /*
    printmessage("Opening file /file1.txt\n");

    fp = fopen("disk0:/file1.txt","r");

    if (!fp){
        printmessage("File not found\n");
        return 1;
    }
    printmessage("File found, reading...\n");

    fread(buff,20,1,fp);

    printmessage("File read:\n");

    printmessage(buff);

    fclose(fp);
    */
    /*while(1){
        sched_yield();
    }*/

    return 0;
}
