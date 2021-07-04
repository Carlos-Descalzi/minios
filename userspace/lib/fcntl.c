#include "unistd.h"
#include "syscall.h"
#include "stdint.h"
#include "path.h"

#define RES_TYPE_RAW    0x00
#define REST_TYPE_FS    0x01

#define DEV_KIND_DISK   0x02

typedef struct {
    uint8_t resource_type;
    uint8_t device_kind:7,
            device_resource:1;
    uint8_t device_instance;
    uint8_t mode;
    const char* path;
} OpenData;

#include "stdio.h"
int open(const char* pathname, int flags){
    char buff[256];
    OpenData open_data = {
        .resource_type = REST_TYPE_FS,
        .device_kind =  DEV_KIND_DISK,
        .device_resource = 1,
        .device_instance = 0,
        .mode = 0,
        .path = path_absolute(pathname, buff)
    };
    return syscall(SYS_OPEN, &open_data);
}

typedef struct {
    unsigned char stream_num;
    void* buff;
    unsigned int size;
} IOData;

size_t read(int fd, void* buff, size_t count){
    IOData read_data = {
        .stream_num = fd,
        .buff = buff,
        .size = count
    };

    return syscall(SYS_READ,&read_data);
}

size_t write(int fd, const void *buff, size_t count){
    IOData write_data = {
        .stream_num = fd,
        .buff = buff,
        .size = count
    };

    return syscall(SYS_WRITE,&write_data);
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
