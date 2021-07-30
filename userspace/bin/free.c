#include "stdio.h"

static void print_stats(int total, int used);

int main(int argc,char **argv){
    char token[16];
    char buffer[256];
    int total;
    int used;
    FILE* fp;

    memset(buffer,0,256);
    memset(token,0,16);

    fp = fopen("sys0:/memory/user","r");

    if (fp){
        fread(buffer,256,1,fp);

        fclose(fp);

        memcpy(token,buffer,index(buffer,','));

        total = atoi(token) / 1024;
        used = atoi(strchr(buffer,',')+1) / 1024;

        printf("User Memory:\n");
        print_stats(total, used);
    }

    memset(buffer,0,256);
    memset(token,0,16);

    fp = fopen("sys0:/memory/kernel","r");

    if (fp){
        fread(buffer,256,1,fp);

        fclose(fp);

        memcpy(token,buffer,index(buffer,','));

        total = atoi(token) / 1024;
        used = atoi(strchr(buffer,',')+1) / 1024;

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
