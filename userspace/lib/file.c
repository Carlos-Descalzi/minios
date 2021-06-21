#include "stdio.h"
#include "fcntl.h"
#include "unistd.h"
#include "stdlib.h"

struct _FILE {
    int fd;
};


FILE _files[] = {
    {.fd = 0 },
    {.fd = 1 },
    {.fd = 2 },
    {.fd = -1 },
    {.fd = -1 },
    {.fd = -1 },
    {.fd = -1 },
    {.fd = -1 },
    {.fd = -1 },
    {.fd = -1 }
};

FILE* stdin = &_files[0];
FILE* stdout = &_files[1];
FILE* stderr = &_files[2];

FILE*   fopen   (const char*pathname, const char* mode){
    for (int i=0;i<10;i++){
        if (_files[i].fd == -1){
            int result = open(pathname, 0);

            if (result >= 0){
                _files[i].fd = result;
                return &(_files[i]);
            }
            break;
        } 
    }
     
    return NULL;
}
int fclose(FILE* stream){
    close(stream->fd);
    stream->fd = -1;
    return 0;
}

size_t fread (void *buff, size_t size, size_t nmemb, FILE* stream){
    return read(stream->fd, buff, size * nmemb);
}

char* fgets(char* buff, int size, FILE* stream){
    int i;
    char c;
    for (i=0;i<size-1;i++){
        read(stream->fd,&c,1);
        if (c != '\n'){
            buff[i] = c;
        } else {
            break;
        }
    }
    buff[i] = '\0';
    return buff;
}
