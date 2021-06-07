#ifndef _FS_H_
#define _FS_H_

#include "fs/ext2.h"

void fs_init(void);
Ext2FileSystem* fs_get_filesystem(Device* device);

#endif
