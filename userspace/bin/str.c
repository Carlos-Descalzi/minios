#include "stdio.h"
#include "string.h"
#include "path.h"

int main(int argc, char** argv){
    /*
    char message[] = "a,b,c,d";
    char* ptr;

    printf("%s\n",strtok_r(message,",",&ptr));
    printf("%s\n",strtok_r(NULL,",",&ptr));
    printf("%s\n",strtok_r(NULL,",",&ptr));
    printf("%s\n",strtok_r(NULL,",",&ptr));
    */
    char buff[256];

    path_absolute("disk0:/bin/../etc",buff);
    printf("%s\n",buff);

    return 0;
}
