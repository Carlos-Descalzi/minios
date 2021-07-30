#include "stdio.h"

static void print_stats(int total, int used);

int main(int argc,char **argv){
    char* token;
    char buffer[256];
    char* p = NULL;
    int total;
    int used;
    FILE* fp;

    memset(buffer,0,256);

    fp = fopen("sys0:/memory/user","r");

    if (fp){
        fread(buffer,256,1,fp);

        fclose(fp);

        token = strtok_r(buffer,",",&p);
        total = atoi(token) / 1024;
        token = strtok_r(NULL,",",&p);
        used = atoi(token) / 1024;

        printf("User Memory:\n");
        print_stats(total, used);
    }

    memset(buffer,0,256);

    fp = fopen("sys0:/memory/kernel","r");

    p = NULL;

    if (fp){
        fread(buffer,256,1,fp);

        fclose(fp);


        token = strtok_r(buffer,",",&p);
        total = atoi(token) / 1024;
        token = strtok_r(NULL,",",&p);
        used = atoi(token) / 1024;

        printf("Kernel Memory:\n");

        print_stats(total, used);
    }

    return 0;
}
static void print_stats(int total, int used){
    char* unit;

    if (total > 1024){
        unit = "MB";
        total /= 1024;
    } else {
        unit ="KB";
    }
    printf("\tTotal:\t%d %s\n", total , unit);

    if (used > 1024){
        unit = "MB";
        total /= 1024;
    } else {
        unit = "KB";
    }
    printf("\tUsed:\t%d %s\n", used, unit);
}
