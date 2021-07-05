#ifndef _MODULES_H_
#define _MODULES_H_

#include "lib/stdint.h"
#include "fs/fs.h"

/**
 * Loads a module from a given file system and path, returning 
 * the entry point address, (the address of the module_register function )
 **/
int32_t modules_load(FileSystem* fs, const char* path);

#endif
