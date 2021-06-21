#include "stdlib.h"
#include "stdio.h"

int main(int argc,char **argv){
    char buff[32];

    printf("Hello world from userspace!!!\n");
    printf("Enter something:");
    fgets(buff,32,stdin);
    printf("You wrote:%s\n",buff);

    return 0;
}
