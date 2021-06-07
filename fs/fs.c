#include "fs/fs.h"
#include "lib/string.h"
#include "misc/debug.h"

static Ext2FileSystem* file_systems[10];

void fs_init(void){
    memset(file_systems,0,sizeof(file_systems));
}

Ext2FileSystem* fs_get_filesystem(Device* device){
    int i;

    for (i=0;i<10;i++){
        if (file_systems[i]){
            debug("fs found\n");
            return file_systems[i];
        } else {
            debug("opening fs\n");
            file_systems[i] = ext2_open(BLOCK_DEVICE(device));
            return file_systems[i];
        }
    }
    return NULL;
}
