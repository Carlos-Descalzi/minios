#include "stdlib.h"
#include "string.h"
#include <stdio.h>

int main(){
    char buff[16];
    //printf("%d\n",3/10);
    printf("%s\n",strcpy(buff,"hola"));
    printf("%s\n",itoa(1234,buff,10));
    return 0;
}
