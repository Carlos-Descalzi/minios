#include "stdio.h"
/**
 * Simple cat command, just dumps the contents of a file into stdout
 **/
int main(int argc, char**argv){
    char buff[1024];

    if (argc < 2){
        return 0;
    }

    FILE* fp = fopen(argv[1],"r");

    if (!fp){
        printf("File not found\n");
        return -1;
    }

    memset(buff,0,1024);

    while (fread(buff,1024,1,fp) > 0){
        printf("%s",buff);
    }

    fclose(fp);

    return 0;
}
