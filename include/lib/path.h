#ifndef _PATH_H_
#define _PATH_H_

#include "lib/stdint.h"

int path_parse(const char* fullpath, uint16_t* device_id, char* filepath);

#endif
