#include "unistd.h"
#include "syscall.h"
#include "stdint.h"
#include "path.h"
#include "stdio.h"

typedef struct {
    uint32_t flags;
    const char* path;
} OpenData;

typedef struct {
    unsigned char stream_num;
    const void* buff;
    unsigned int size;
} IOData;

int open(const char* pathname, int flags){
    char buff[256];
    OpenData open_data = {
        .flags = flags,
        .path = path_absolute(pathname, buff)
    };
    return syscall(SYS_OPEN, &open_data);
}

ssize_t read(int fd, void* buff, size_t count){
    IOData read_data = {
        .stream_num = fd,
        .buff = buff,
        .size = count
    };

    return (ssize_t) syscall(SYS_READ, &read_data);
}

ssize_t write(int fd, const void *buff, size_t count){
    IOData write_data = {
        .stream_num = fd,
        .buff = buff,
        .size = count
    };

    return (ssize_t) syscall(SYS_WRITE, &write_data);
}

int close(int fd){
    return syscall(SYS_CLOSE, (void*)fd);
}

int stat(const char* pathname, struct stat* statbuf){
    char buff[256];
    struct {
        const char* pathname;
        struct stat* statbuf;
    } stat_data = {
        .pathname = path_absolute(pathname,buff),
        .statbuf = statbuf
    };

    return syscall(SYS_STAT,&stat_data);
}
