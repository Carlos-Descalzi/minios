#include "stdio.h"
#include "stdlib.h"

int main(int argc, char** argv){

    char* data = malloc(100);

    sprintf(data, "hello world!\n");

    printf("%s\n", data);

    free(data);

    return 0;
}
