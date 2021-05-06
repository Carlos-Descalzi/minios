#include "ext2.h"
#include "stdint.h"


#define BUF_LENGHT  1024
static char block_buffer[BUF_LENGHT];

static char*    ext2_path_tokenize  (const char* path, char* buffer, uint8_t pos);
static int16_t  ext2_find_inode     (Ext2FileSystem* fs, const char* path, Ext2Inode* inode);
static int16_t  ext2_iter_dir_block (Ext2FileSystem* fs, uint32_t block_num, ListIterator iterator, void* func_data);
static void     ext2_locate_inode   (Ext2FileSystem* fs, uint32_t inode_num, uint32_t* block_number, uint32_t* block_position);
static void     ext2_list_dir_inode (Ext2FileSystem* fs, Ext2Inode* inode, ListIterator iterator, void* func_data);

int16_t ext2_open(Ext2FileSystem* fs,BlockDevice* device){
    if (!fs){
        return 0;
    }
    block_device_seek(device, 1024);
    block_device_read(device, (char*)&(fs->super_block), BUF_LENGHT);
    fs->device = device;
    fs->block_size = 1024 << fs->super_block.log_block_size;
    return 1;
}

void ext2_list(Ext2FileSystem* fs, const char* path, ListIterator iterator, void* func_data){
    Ext2Inode inode;

    if (!fs || !iterator){
        return;
    }

    if (ext2_find_inode(fs, path, &inode)){
        ext2_list_dir_inode(fs, &inode, iterator, func_data);
    }

}
void ext2_close(Ext2FileSystem* fs){
}

static int16_t ext2_find_inode(Ext2FileSystem* fs, const char* path, Ext2Inode* inode){
    char path_token[256];
    uint8_t pos=0;

    while(ext2_path_tokenize(path,path_token,&pos)){
    }
}

static char* ext2_path_tokenize(const char* path, char* buffer, uint8_t* pos){

    if (path[*pos] == '/'){
        buffer[0] = '/';
        buffer[1] = '\0';
        (*pos)++;
        return buffer;
    } else {
        int i;
        for (i=0;path[*pos] && path[*pos] != '/';i++,(*pos)++){
            buffer[i] = path[*pos];
        }
        buffer[i] = '\0';
        (*pos)++;
        return buffer;
    }
    return NULL;
}
static void ext2_locate_inode(Ext2FileSystem* fs, uint32_t inode_num, uint32_t* block_number, uint32_t* block_position){
    *block_number = (inode_num - 1) / inodes_per_group;
    *block_position = (inode_num -1) % inodes_per_group;
}
static void ext2_list_dir_inode(Ext2FileSystem* fs, Ext2Inode* inode, ListIterator iterator, void* func_data){
    for (i=0;i<15;i++){
        uint32_t block = inode->blocks[i];
        if (ext2_iter_dir_block(fs, block, iterator, func_data)){
            return;
        }
    }
}
static int16_t ext2_iter_dir_block(Ext2FileSystem* fs, uint32_t block_num, ListIterator iterator, void* func_data){
    Ext2Dirent* dir_entry;
    block_device_seek(fs->device, block_num);
    block_device_read(fs->device, block_buffer, BUF_LENGHT);

    for(dir_entry = (Dirent*)block_buffer;
        dir_entry->file_type != EXT_FT_UNKNOWN;
        dir_entry = (DirEntry*)((char*)dir_entry)+dir_entry->rec_len){

        if (iterator(fs, dir_entry, func_data)){
            return 1;
        }
    }
    return 0;

}
