#ifndef _DIRENT_H_
#define _DIRENT_H_

#define DT_BLK      0x00
#define DT_CHR      0x01
#define DT_DIR      0x02
#define DT_FIFO     0x03
#define DT_LNK      0x04
#define DT_REG      0x05
#define DT_SOCK     0x06
#define DT_UNKNOWN  0x07

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
