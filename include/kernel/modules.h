#ifndef _MODULES_H_
#define _MODULES_H_

#include "lib/stdint.h"
#include "fs/ext2.h"

int32_t modules_load(Ext2FileSystem* fs, const char* path);

#endif
