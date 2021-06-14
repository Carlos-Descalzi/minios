#include "stdlib.h"
#include "sched.h"
#include "stdio.h"
/**
 * User space initialization program
 **/
int main(int argc,char **argv){
    char buff[32];
    FILE* fp;
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
    /*while(1){
        sched_yield();
    }*/

    return 0;
}
