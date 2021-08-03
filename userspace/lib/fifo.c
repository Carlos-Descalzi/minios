#include "sys/stat.h"
#include "syscall.h"
#include "stdint.h"

int mkfifo(const char* pathname, mode_t mode){
    struct {
        char* pathname;
        uint32_t mode;
    } fifo_data = {
        .pathname = (char*) pathname,
        .mode = mode
    };

    return syscall(SYS_PIPE, &fifo_data);
}
