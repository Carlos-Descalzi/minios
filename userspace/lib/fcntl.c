#include "unistd.h"
#include "syscall.h"
#include "stdint.h"

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

int open(const char* pathname, int flags){
    OpenData open_data = {
        .resource_type = REST_TYPE_FS,
        .device_kind =  DEV_KIND_DISK,
        .device_resource = 1,
        .device_instance = 0,
        .mode = 0,
        .path = pathname
    };
    return syscall(SYS_OPEN, &open_data);
}

typedef struct {
    unsigned char stream_num;
    void* buff;
    unsigned int size;
} ReadData;

size_t read(int fd, void* buff, size_t count){
    ReadData read_data = {
        .stream_num = fd,
        .buff = buff,
        .size = count
    };

    return syscall(SYS_READ,&read_data);
}

int close(int fd){
    return syscall(SYS_CLOSE, (void*)fd);
}

