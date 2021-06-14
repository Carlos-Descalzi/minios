#ifndef _UNISTD_H_
#define _UNISTD_H_

#include "stddef.h"

size_t read(int fd, void* buf, size_t count);
int close(int fd);

#endif
