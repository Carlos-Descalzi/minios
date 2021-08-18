#include "stdio.h"

int main(int argc, char** argv){

    FILE* fp = fopen("ser0:","r+");

    char buff;

    while(1){
        fread(&buff,1,1,fp);
        printf("%c",buff);
    }
    return 0;
}
