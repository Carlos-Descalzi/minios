#include "fs/fs.h"
#include "lib/string.h"
#include "misc/debug.h"

#define FS_MAX  10

typedef struct {
    FileSystem* file_system;
    int ref_count;
} FsSlot;

static FileSystemType* types[FS_MAX];
static FsSlot file_systems[FS_MAX];

static FileSystem* create(BlockDevice* device);

void fs_init(void){
    memset(file_systems,0,sizeof(file_systems));
    memset(types,0,sizeof(types));
}

void fs_register_type(FileSystemType* fs_type){
    for (int i=0;i<FS_MAX;i++){
        if (!types[i]){
            types[i] = fs_type;
            break;
        }
    }
}

FileSystem* fs_get_filesystem(BlockDevice* device){
    int i;

    for (i=0;i<FS_MAX;i++){
        if (file_systems[i].file_system &&
            file_systems[i].file_system->device == device){
            file_systems[i].ref_count++;
            return file_systems[i].file_system;
        }
    }

    for (i=0;i<FS_MAX;i++){
        if (!file_systems[i].file_system){
            file_systems[i].file_system = create(device);

            if (file_systems[i].file_system){
                file_systems[i].ref_count = 1;
                return file_systems[i].file_system;
            }
            debug("Unable to create filesystem\n");
            return NULL;
        }
    }
    return NULL;
}

void fs_release_filesystem(FileSystem* fs){
    for (int i=0;i<FS_MAX;i++){
        if (file_systems[i].file_system == fs){
            file_systems[i].ref_count--;

            if (file_systems[i].ref_count == 0){
                //fs_close(file_systems[i].file_system);
                //file_systems[i].file_system = NULL;
                fs_release_resources(fs);
            }
            break;
        }
    }
}

static FileSystem* create(BlockDevice* device){
    debug("Instantiating fs for device\n");
    for (int i=0;i<FS_MAX;i++){
        if (types[i]){
            debug("Trying ");debug(types[i]->type_name);debug("\n");
            FileSystem* fs = fs_type_create(types[i], device);

            if (fs){
                return fs;
            }
        }
    }

    return NULL;
}

Stream* fs_open_stream_path (FileSystem* fs, const char* path, uint32_t flags){

    uint32_t inodenum = fs_find_inode(fs, path);

    if (inodenum){
        return fs_open_stream(fs, inodenum, flags);
    }

    return NULL;
}
