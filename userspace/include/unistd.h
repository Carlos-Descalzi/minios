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


#define S_IFMT      0170000
#define S_IFSOCK    0140000
#define S_IFLNK     0120000
#define S_IFREG     0100000
#define S_IFBLK     0060000
#define S_IFDIR     0040000
#define S_IFCHR     0020000
#define S_IFIFO     0010000

#define S_IRUSR     0000400
#define S_IWUSR     0000200
#define S_IXUSR     0000100
#define S_IRGRP     0000040
#define S_IWGRP     0000020
#define S_IXGRP     0000010
#define S_IROTH     0000004
#define S_IWOTH     0000002
#define S_IXOTH     0000001

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
