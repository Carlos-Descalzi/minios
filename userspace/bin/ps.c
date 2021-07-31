#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "dirent.h"

int main(int argc, char** argv){
    char path[40];
    char buffer[256];

    struct dirent* entry = NULL;

    DIR* dir = opendir("sys0:/processes");

    if (!dir){
        return 1;
    }

    printf("PID\tCMD\n");

    while ((entry = readdir(dir))){
        if (!strcmp(entry->d_name,".")
            || !strcmp(entry->d_name, "..")){
            continue;
        }
        printf("%s\t",entry->d_name);

        sprintf(path,"sys0:/processes/%s",entry->d_name);
        FILE* fp = fopen(path,"r");

        if (fp){
            if(strlen(fgets(buffer, 256, fp))){
                printf("%s",buffer);
                fclose(fp);
            }
        }

        printf("\n");
    }

    closedir(dir);


    return 0;
}
