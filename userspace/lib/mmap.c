#include "mman.h"
#include "syscall.h"
#include "stdint.h"

typedef struct {
    uint32_t length;
    uint32_t prot;
    uint32_t flags;
    uint32_t fd;
    uint32_t offset;
} MmapOp;

void* mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset){
    MmapOp op = {
        .length = length,
        .prot = prot,
        .flags = flags,
        .fd = fd,
        .offset = offset
    };

    uint32_t ret_addr = syscall(SYS_MMAP, &op);

    if (ret_addr != ((uint32_t)-1)){
        return (void*) ret_addr;
    }
    return NULL;
}
