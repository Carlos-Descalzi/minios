#ifndef _UNISTD_H_
#define _UNISTD_H_

#include "stddef.h"
#include "stdint.h"

#define EAGAIN  -10 
#define EBADF   -11  // invalid descritor or not open for reading
#define EFAULT  -12  // buffer not in accessible space
#define EINTR   -13  // interrupted by a signal
#define EINVAL  -14  // not valid for reading
#define EISDIR  -15  // is directory
#define EIO     -16  // io error

struct stat {
    dev_t       st_dev;
    uint32_t    st_ino;
    uint32_t    st_mode;
    uint32_t    st_nlink;
    uint32_t    st_uid;
    uint32_t    st_gid;
    dev_t       st_rdev;
    uint32_t    st_size;
    uint32_t    st_blksize;
    uint32_t    st_blocks;
    uint32_t    st_atim;
    uint32_t    st_mtim;
    uint32_t    st_ctim;
};

ssize_t     read    (int fd, void* buf, size_t count);
ssize_t     write   (int fd, const void *buf, size_t count);
int         close   (int fd);
int         stat    (const char* pathname, struct stat* statbuf);
pid_t       getpid  (void);

#endif
