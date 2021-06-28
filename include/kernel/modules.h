#ifndef _MODULES_H_
#define _MODULES_H_

#include "lib/stdint.h"
#include "fs/fs.h"

int32_t modules_load(FileSystem* fs, const char* path);

#endif
