#include "stdio.h"

int main(int argc,char **argv){

    FILE* fp = fopen("net0:","w+");

    if (!fp){
        printf("Unable to open device\n");
        return 1;
    }

    int ret = fwrite("test",4,1,fp);

    printf("Write returned %d\n",ret);

    return 0;
}
