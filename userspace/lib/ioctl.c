#include "sys/ioctl.h"
#include "syscall.h"
#include "stdint.h"

typedef struct {
    uint32_t fd;
    uint32_t request;
    void* data;
} IoctlOp;

int ioctl(int fd, unsigned long request, void* data){
    IoctlOp op = {
        .fd = fd,
        .request = (uint32_t) request,
        .data = data
    };
    return syscall(SYS_IOCTL, &op);
}
