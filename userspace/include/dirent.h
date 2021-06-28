#ifndef _DIRENT_H_
#define _DIRENT_H_

#include "stdint.h"

#define DT_UNKNOWN  0
#define DT_REG      1
#define DT_DIR      2
#define DT_CHR      3
#define DT_BLK      4
#define DT_FIFO     5
#define DT_SOCK     6
#define DT_LNK      7
#define DT_WTH      8

typedef struct DIR_ DIR;

struct dirent {
    uint32_t d_ino;
    uint32_t d_off;
    uint16_t d_reclen;
    uint8_t  d_type;
    char     d_name[256];
};

DIR*            opendir     (const char* path);
struct dirent*  readdir     (DIR* dirp);
int             closedir    (DIR* dirp);

#endif
