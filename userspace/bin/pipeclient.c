#include "stdio.h"
#include "fcntl.h"
#include "unistd.h"
#include "string.h"
#include "stdlib.h"

int main(int argc, char** argv){
    char fname[30];
    char message[] = "Hola mundo!";

    if (argc < 2){
        printf("Missing pid\n");
        return 1;
    }

    int pid = atoi(argv[1]);

    sprintf(fname,"sys0:/pipes/pipe%d", pid);

    int fd = open(fname, O_WRONLY);

    write(fd, message, strlen(message));

    close(fd);

    return 0;
}
