#include "stdio.h"
#include "fcntl.h"
#include "unistd.h"
#include "stdlib.h"
#include "string.h"

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

static int mode_to_flags(const char* mode);

FILE* fopen(const char*pathname, const char* mode){
    for (int i=0;i<10;i++){
        if (_files[i].fd == -1){
            int result = open(pathname, mode_to_flags(mode));

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
    for (i=0;i<size-1;){
        if (read(stream->fd,&c,1) > 0){
            if (c == '\b'){
                if (i > 0){
                    i--;
                }
            } else if (c != '\n'){
                buff[i++] = c;
            } else {
                break;
            }
        } else {
            break;
        }
    }
    buff[i] = '\0';
    return buff;
}

size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream){
    return write(stream->fd, ptr, size * nmemb);
}
int fgetc (FILE* stream){
    char c;
    if (read(stream->fd,&c,1) > 0){
        return c;
    }
    return EOF;
}

static int mode_to_flags(const char* mode){
    int flags = 0;

    if (strchr(mode,'w')){
        flags |= O_WRONLY;
        if (strchr(mode,'+')){
            flags |= O_RDONLY;
        }
    }
    if (strchr(mode,'r')){
        flags |= O_RDONLY;
        if (strchr(mode, '+')){
            flags |= O_WRONLY;
        }
    }
    if (strchr(mode,'a')){
        flags |= O_APPEND | O_WRONLY;
        if (strchr(mode,'+')){
            flags |= O_RDONLY;
        }
    }

    return flags;
}
int fseek (FILE* fp, long offset, int whence){
    lseek(fp->fd, offset, whence);
}
void rewind (FILE *stream){
    fseek(stream,0,SEEK_SET);
}
long ftell (FILE* fp){
    return lseek(fp->fd, 0, SEEK_CUR);
}
int fflush (FILE *stream){
    // TODO implement
    return 0;
}
int remove (const char* path){
    // TODO implement
    return 0;
}
int ferror (FILE *stream){
    // TODO implement
    return 0;
}
int ungetc (int c, FILE* fp){
    // TODO implement
    return 0;
}
