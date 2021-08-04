#include "stdio.h"
#include "sys/stat.h"
#include "unistd.h"
#include "string.h"

int main(int argc, char** argv){
    char fname[30];
    char buffer[100];

    int pid = getpid();

    sprintf(fname,"sys0:/pipes/pipe%d", pid);

    int fd = mkfifo(fname, 0);

    if (fd <= 0){
        printf("Unable to create pipe\n");
        return 1;
    }

    while(1){
        memset(buffer,0, 100);
        read(fd, buffer, 100);

        printf("Received: %s\n", buffer);
    }

    return 0;
}
