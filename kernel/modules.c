#include "kernel/modules.h"
#include "io/streams.h"
#include "misc/debug.h"
#include "kernel/paging.h"

int32_t modules_load(FileSystem* fs, const char* path){
    debug("Loading module ");debug(path);debug(" ...\n");
    Stream* stream = fs_open_stream_path(fs, path, O_RDONLY);

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
