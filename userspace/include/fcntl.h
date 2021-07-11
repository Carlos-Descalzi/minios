#ifndef _FCNTL_H_
#define _FCNTL_H_

#define O_RDONLY        0x01
#define O_WRONLY        0x02
#define O_RDWR          0x03

#define O_CREAT         0x04
#define O_APPEND        0x08
#define O_TRUNC         0x10
#define O_NONBLOCK      0x20

int open(const char* pathname, int flags);

#endif
