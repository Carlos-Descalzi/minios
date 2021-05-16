#include "fs/ext2.h"
#include "lib/stdint.h"
#include "lib/heap.h"
#include "lib/string.h"
#include "misc/debug.h"


#define OFFSET  (512 + 65536 + 1024)   // boot sector + kernel
#define BUF_LENGHT  1024

static char*    ext2_path_tokenize  (const char* path, char* buffer, uint8_t* pos);
static int16_t  ext2_find_inode     (Ext2FileSystem* fs, const char* path, Ext2Inode* inode);
static int16_t  ext2_iter_dir_block (Ext2FileSystem* fs, uint32_t block_num, DirVisitor visitor, void* func_data);
static void     ext2_locate_inode   (Ext2FileSystem* fs, uint32_t inode_num, uint32_t* block_number, uint32_t* block_position);
static void     ext2_list_dir_inode (Ext2FileSystem* fs, Ext2Inode* inode, DirVisitor visitor, void* func_data);

static inline void ext2_device_seek(Ext2FileSystem* fs, uint32_t pos){
    block_device_seek(fs->device, pos + OFFSET); // skip boot sector + kernel
}
static inline void ext2_device_read_block(Ext2FileSystem*fs){
    block_device_read(fs->device, fs->block_buffer, fs->block_size);
}
static inline void ext2_device_gotoblock(Ext2FileSystem *fs, uint32_t block){
    ext2_device_seek(fs, fs->block_size*block);
}

Ext2FileSystem* ext2_open(BlockDevice* device){
    Ext2FileSystem* fs;
    if (!device){
        debug("No device\n");
        return NULL;
    }
    fs = heap_alloc(sizeof(Ext2FileSystem));
    fs->device = device;
    ext2_device_seek(fs, 1024); // skip boot sector + kernel
    debug("Reading superblock\n");
    block_device_read(device, (uint8_t*)&(fs->super_block), BUF_LENGHT);
    fs->block_size = 1024 << fs->super_block.log_block_size;
    fs->block_buffer = heap_alloc(fs->block_size);
    debug("Ext2 Block size: "); debug_i(fs->block_size,10); debug("\n");
    debug("Inode size:"); debug_i(fs->super_block.inode_size,10); debug("\n");
    debug("Inode count:"); debug_i(fs->super_block.inode_count,10); debug("\n");
    debug("Block count:"); debug_i(fs->super_block.block_count,10); debug("\n");
    return fs;
}
void ext2_list_inodes(Ext2FileSystem* fs, InodeVisitor visitor, void*data){
    int n,i,j;
    uint32_t pos;
    int nentries = fs->block_size / sizeof(Ext2BlockGroupDescriptor);
    int ninodes = fs->block_size / sizeof(Ext2Inode);
    Ext2BlockGroupDescriptor descriptors[nentries];
    Ext2Inode* inode_table;

    pos = fs->block_size == 1024 ? fs->block_size * 2 : fs->block_size;
    ext2_device_seek(fs, pos);

    //for(n=0;n<fs->super_block.blocks_per_group;n++){
    while(1){
        ext2_device_read_block(fs);
        memcpy(descriptors,fs->block_buffer,fs->block_size);

        for (i=0;i< nentries;i++){
            if (descriptors[i].inode_table == 0){
                return;
            }
            debug("Inode table at ");
            debug_i(descriptors[i].inode_table,10);
            debug("\n");
            ext2_device_gotoblock(fs, descriptors[i].inode_table);
            inode_table = (Ext2Inode*)fs->block_buffer;

            for (j=0;j<ninodes;j++){
                if (visitor(fs, &(inode_table[j]),data)){
                    return;
                }
            }

        }
    }
}

void ext2_list(Ext2FileSystem* fs, const char* path, DirVisitor visitor, void* func_data){
    Ext2Inode inode;

    if (!fs || !visitor){
        return;
    }

    if (ext2_find_inode(fs, path, &inode)){
        ext2_list_dir_inode(fs, &inode, visitor, func_data);
    }

}
void ext2_close(Ext2FileSystem* fs){
    heap_free(fs->block_buffer);
    heap_free(fs);
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
    //*block_number = (inode_num - 1) / inodes_per_group;
    //*block_position = (inode_num -1) % inodes_per_group;
}
static void ext2_list_dir_inode(Ext2FileSystem* fs, Ext2Inode* inode, DirVisitor visitor, void* func_data){
    int i;
    for (i=0;i<15;i++){
        uint32_t block = inode->blocks[i];
        if (ext2_iter_dir_block(fs, block, visitor, func_data)){
            return;
        }
    }
}
static int16_t ext2_iter_dir_block(Ext2FileSystem* fs, uint32_t block_num, DirVisitor visitor, void* func_data){
    Ext2DirEntry* dir_entry;
    block_device_seek(fs->device, block_num);
    block_device_read(fs->device, fs->block_buffer, BUF_LENGHT);

    for(dir_entry = (Ext2DirEntry*)fs->block_buffer;
        dir_entry->file_type != EXT2_DIR_ENTRY_UNKNOWN;
        dir_entry = (Ext2DirEntry*)((char*)dir_entry)+dir_entry->rec_len){

        if (visitor(fs, dir_entry, func_data)){
            return 1;
        }
    }
    return 0;

}
