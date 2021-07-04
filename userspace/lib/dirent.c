#include "dirent.h"
#include "unistd.h"
#include "syscall.h"
#include "stdio.h"
#include "string.h"

struct DIR_ {
    uint32_t inode;
    uint16_t device_id;
    int32_t last_entry_offset;
};

static DIR dirs[] = {
    { .inode=((uint32_t)-1) },
    { .inode=((uint32_t)-1) },
    { .inode=((uint32_t)-1) },
    { .inode=((uint32_t)-1) },
    { .inode=((uint32_t)-1) },
    { .inode=((uint32_t)-1) },
    { .inode=((uint32_t)-1) },
    { .inode=((uint32_t)-1) },
    { .inode=((uint32_t)-1) },
    { .inode=((uint32_t)-1) }
};

static struct dirent dir_entry;

#define DIRS_MAX    (sizeof(dirs)/sizeof(DIR))

DIR* opendir(const char* path){
    DIR* dir = NULL;
    struct stat statb;

    for (int i=0;i<DIRS_MAX;i++){
        if (dirs[i].inode == ((uint32_t)-1)){
            dir = &dirs[i];
            break;
        }
    }

    if (!dir){
        return NULL;
    }

    int ret = stat(path, &statb);
    if (ret){
        return NULL;
    }

    dir->inode = statb.st_ino;
    dir->device_id = statb.st_dev;
    dir->last_entry_offset = 0;

    return dir;
}

int closedir(DIR* dir){

    dir->inode = (uint32_t)-1;
    dir->device_id = (uint16_t)-1;
    dir->last_entry_offset = 0;
    return 0;
}

typedef struct {
    uint16_t device_id;
    uint32_t offset_next;
    uint32_t inode;
    uint16_t rec_len;
    uint8_t name_len;
    uint8_t file_type;
    char name[256];
} SysDirEntry;

struct dirent* readdir(DIR* dirp){
    if (dirp->last_entry_offset == -1){
        return NULL;
    }
    SysDirEntry entry = {
        .device_id = dirp->device_id,
        .inode = dirp->inode,
        .offset_next = dirp->last_entry_offset
    };

    int result = syscall(SYS_GETDENT, &entry);

    if (result < 0){
        return NULL;
    }

    if (result == 1){
        dirp->last_entry_offset = -1;
    } else {
        dirp->last_entry_offset = entry.offset_next;
    }
    dir_entry.d_ino = entry.inode;
    dir_entry.d_off = entry.offset_next;
    dir_entry.d_reclen = entry.rec_len;
    dir_entry.d_type = entry.file_type;
    strcpy(dir_entry.d_name, entry.name);

    return &dir_entry;
}













