#include "stdio.h"
#include "string.h"


int main(int argc, char** argv){
    char buff[256];

    printf("Mini-shell v0.1\n\n");

    while(1){
        printf("$ ");
        fgets(buff,256,stdin);

        if (strlen(buff) > 0){
            if (!strncmp(buff,"ls",2)){
            } else if (!strncmp(buff,"clear",5)){
                printf("\e[2J");
            } else if (!strncmp(buff,"ps",3)){
            }
        }
    }



    return 0;
}
