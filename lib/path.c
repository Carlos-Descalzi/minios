#include "lib/path.h"
#include "lib/string.h"
#include "kernel/device.h"


int path_parse(const char* fullpath, uint16_t* device_id, char* filepath){
    char devname[20];

    if (!fullpath){
        return -1;
    }

    char* pos = strchr(fullpath,':');

    if (!pos){
        return -2;
    }
    strncpy(devname, fullpath, ((uint32_t)pos)-((uint32_t)fullpath));

    uint8_t kind;
    uint8_t instance;

    if(device_parse_name(devname, &kind, &instance)){
        return -2;
    }

    *device_id = device_make_id(kind, instance);

    strcpy(filepath, pos+1);

    return 0;
}
