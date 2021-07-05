#include "stdio.h"
#include "syscall.h"
#include "string.h"
/**
 * This proram lists all available devices instantiated
 * in the system
 **/

int main(int argc, char** argv){
    
    struct {
        int index;
        char buffer[30];
    } get_dev_data;

    unsigned int ret = 0;
    memset(&get_dev_data,0,sizeof(get_dev_data));

    printf("Available devices:\n\n");
    do {
        ret = syscall(SYS_DEVS,&get_dev_data);
        if (!ret){
            printf(" - %s\n",get_dev_data.buffer);
            get_dev_data.index++;
        }
    } while (ret == 0);
    

    return 0;
}
