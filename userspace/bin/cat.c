#include "stdio.h"

int main(int argc, char**argv){
    char buff[1024];

    if (argc == 0){
        return 0;
    }

    FILE* fp = fopen(argv[0],"r");

    if (!fp){
        printf("File not found\n");
        return -1;
    }

    while (fread(buff,1024,1,fp) > 0){
        printf("%s",buff);
    }

    fclose(fp);

    return 0;
}
