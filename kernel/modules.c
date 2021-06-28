#include "kernel/modules.h"
#include "io/streams.h"
#include "misc/debug.h"

int32_t modules_load(Ext2FileSystem* fs, const char* path){
    debug("Loading module ");debug(path);debug(" ...\n");
    Stream* stream = ext2_file_stream_open(fs, path,0);

    if (!stream){
        debug("Module not found\n");
        return -1;
    }
    void(*function)(void) = (void*)paging_kernel_load_code(stream);

    if (!function){
        debug("Module not loaded\n");
        return -2;
    }

    function();
    debug("Module ");debug(path);debug(" loaded\n");

    return 0;
}
