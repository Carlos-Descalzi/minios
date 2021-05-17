#include "fs/ext2.h"
#include "lib/stdint.h"
#include "lib/heap.h"
#include "lib/string.h"
#include "lib/stdlib.h"
#include "misc/debug.h"


#define OFFSET      (512 + 65536 +1024)   // boot sector + kernel

//static char*    ext2_path_tokenize  (const char* path, char* buffer, uint8_t* pos);
//static int16_t  ext2_find_inode     (Ext2FileSystem* fs, const char* path, Ext2Inode* inode);
static int16_t  ext2_iter_dir_block (Ext2FileSystem* fs, uint32_t block_num, DirVisitor visitor, void* func_data);
//static void     ext2_locate_inode   (Ext2FileSystem* fs, uint32_t inode_num, uint32_t* block_number, uint32_t* block_position);
static void     ext2_list_dir_inode (Ext2FileSystem* fs, Ext2Inode* inode, DirVisitor visitor, void* func_data);

#define ext2_device_seek(fs, pos) { \
    block_device_seek(fs->device, pos + OFFSET); \
} 
#define ext2_device_read_block(fs){ \
    block_device_read(fs->device, fs->block_buffer, fs->block_size); \
}
#define ext2_device_gotoblock(fs, block){ \
    ext2_device_seek(fs, fs->block_size*block); \
}
#define ext2_device_pos(fs) (block_device_pos(fs->device) - OFFSET)

Ext2FileSystem* ext2_open(BlockDevice* device){
    Ext2FileSystem* fs;
    debug("EXT2 - Opening ext2 filesystem\n");
    if (!device){
        debug("No device\n");
        return NULL;
    }
    fs = heap_alloc(sizeof(Ext2FileSystem));
    block_device_seek(device, OFFSET + 1024); 
    debug("EXT2 - Reading superblock "); debug_i(sizeof(Ext2Superblock),10);debug("\n");
    block_device_read(device, (uint8_t*)&(fs->super_block), sizeof(Ext2Superblock));

    if (fs->super_block.magic != 0xef53){
        debug("EXT2 - Bad magic number ("); debug_i(fs->super_block.magic,16); debug(") - no ext2 filesyste found\n");
        heap_free(fs);
        return NULL;
    } else {
        debug("EXT2 - Magic number found\n");
    }
    fs->device = device;
    fs->block_size = 1024 << fs->super_block.log_block_size;
    fs->block_buffer = heap_alloc(fs->block_size);
    fs->first_block_group_pos = fs->block_size * (fs->block_size == 1024 ? 2 : 1);
    debug("EXT2 - Ext2 Block size: "); debug_i(fs->block_size,10); debug("\n");
    debug("EXT2 - Inode size:"); debug_i(fs->super_block.inode_size,10); debug("\n");
    debug("EXT2 - Inode count:"); debug_i(fs->super_block.inode_count,10); debug("\n");
    debug("EXT2 - Block count:"); debug_i(fs->super_block.block_count,10); debug("\n");
    debug("EXT2 - Blocks per group:"); debug_i(fs->super_block.blocks_per_group, 10); debug("\n");
    debug("EXT2 - Free Inode count:"); debug_i(fs->super_block.free_inode_count,10); debug("\n");
    debug("EXT2 - Free Block count:"); debug_i(fs->super_block.free_block_count,10); debug("\n");
    return fs;
}
static uint32_t get_nblock_groups(Ext2FileSystem* fs){
    return 1 + (fs->super_block.block_count / fs->super_block.blocks_per_group);
}
void ext2_list_inodes(Ext2FileSystem* fs, InodeVisitor visitor, void*data){
    uint32_t nentries = fs->block_size / sizeof(Ext2BlockGroupDescriptor);
    uint32_t ninodes = fs->block_size / sizeof(Ext2Inode);
    uint32_t ngroups = get_nblock_groups(fs);
    Ext2BlockGroupDescriptor* descriptors = heap_alloc(sizeof(Ext2BlockGroupDescriptor) * nentries);
    Ext2Inode* inode_table = heap_alloc(sizeof(Ext2Inode) * ninodes);
    int i,j;
    uint32_t pos;
    uint32_t inodenum = 1;
    uint32_t ninodetableblocks = fs->super_block.blocks_per_group / ninodes;

    debug("EXT2 - Number of block groups:"); debug_i(ngroups, 10); debug("\n");
    debug("EXT2 - Number of inodes per block:"); debug_i(ninodes,10); debug("\n");
    debug("EXT2 - Number of blocks for inode table:"); debug_i(ninodetableblocks,10); debug("\n");

    pos = fs->first_block_group_pos;

#define __exit {heap_free(inode_table);heap_free(descriptors);} return

    while(ngroups > 0){
        ext2_device_seek(fs, pos);
        ext2_device_read_block(fs);
        pos = ext2_device_pos(fs);
        memcpy(descriptors,fs->block_buffer,fs->block_size);

        for (i=0;i< nentries && ngroups > 0;i++,ngroups--){
            if (descriptors[i].inode_table == 0){
                __exit;
            }

            debug("EXT2 - Block ");debug_i(i,10);debug(", inode table ");
            debug_i(descriptors[i].inode_table,10); 
            debug(", n dirs:");debug_i(descriptors[i].used_dirs_count,10);
            debug(", used blocks:");debug_i(fs->super_block.blocks_per_group - descriptors[i].free_blocks_count,10);
            debug(", free blocks:");debug_i(descriptors[i].free_blocks_count,10);debug("\n");

            ext2_device_gotoblock(fs, descriptors[i].inode_table);
            for(; ninodetableblocks > 0;ninodetableblocks--){
                ext2_device_read_block(fs);
                pos = ext2_device_pos(fs);
                memcpy(inode_table,fs->block_buffer,sizeof(Ext2Inode)*ninodes);

                for (j=0;j<ninodes;j++,inodenum++){
                    if (visitor(fs, inodenum, &(inode_table[j]),data)){
                        __exit;
                    }
                }
                ext2_device_seek(fs, pos);
            }
        }
    }
    __exit;
}
void ext2_list_directory (Ext2FileSystem* fs, Ext2Inode* inode, DirVisitor visitor, void* data){
    ext2_list_dir_inode(fs, inode, visitor, data);
}

void ext2_close(Ext2FileSystem* fs){
    heap_free(fs->block_buffer);
    heap_free(fs);
}
uint32_t ext2_find_inode(Ext2FileSystem* fs, const char* path){
    uint32_t current_inode = 2; // root directory inode
}
/*
void ext2_list(Ext2FileSystem* fs, const char* path, DirVisitor visitor, void* func_data){
    Ext2Inode inode;

    if (!fs || !visitor){
        return;
    }

    if (ext2_find_inode(fs, path, &inode)){
        ext2_list_dir_inode(fs, &inode, visitor, func_data);
    }

}
static int16_t ext2_find_inode(Ext2FileSystem* fs, const char* path, Ext2Inode* inode){
    char path_token[256];
    uint8_t pos=0;

    while(ext2_path_tokenize(path,path_token,&pos)){
    }
    return 0;
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
    // *block_number = (inode_num - 1) / inodes_per_group;
    // *block_position = (inode_num -1) % inodes_per_group;
}
*/

static void ext2_list_dir_blocks(Ext2FileSystem* fs, uint32_t* blocks, uint32_t nblocks, DirVisitor visitor, void* data){
    int i;
    for (i=0;i<nblocks;i++){
        uint32_t block = blocks[i];
        if (block != 0){
            //debug("EXT2 - Inode block ");debug_i(block,10);debug("\n");
            if (ext2_iter_dir_block(fs, block, visitor, data)){
                return;
            }
        }
    }
}

static void ext2_list_dir_inode(Ext2FileSystem* fs, Ext2Inode* inode, DirVisitor visitor, void* data){
    uint32_t *blocks;
    uint32_t *iblocks;
    uint32_t *i2blocks;
    int i,j;
    debug("EXT2 - Direct blocks\n");
    ext2_list_dir_blocks(fs,inode->blocks,12, visitor, data);
    if (inode->blocks[12]){
        debug("Simply indirect block found\n");
        blocks = heap_alloc(fs->block_size);
        ext2_device_gotoblock(fs, inode->blocks[12]);
        ext2_device_read_block(fs);
        memcpy(blocks,fs->block_buffer,fs->block_size);
        ext2_list_dir_blocks(fs, blocks, fs->block_size / sizeof(uint32_t), visitor, data);
        heap_free(blocks);
    }
    if (inode->blocks[13]){
        debug("Double indirect block found\n");
        blocks = heap_alloc(fs->block_size);
        iblocks = heap_alloc(fs->block_size);
        ext2_device_gotoblock(fs, inode->blocks[12]);
        ext2_device_read_block(fs);
        memcpy(blocks,fs->block_buffer,fs->block_size);
        for (i=0;i<fs->block_size / sizeof(uint32_t);i++){
            if (blocks[i]) {
                ext2_device_gotoblock(fs, blocks[i]);
                ext2_device_read_block(fs);
                memcpy(iblocks,fs->block_buffer,fs->block_size);
                ext2_list_dir_blocks(fs, iblocks, fs->block_size / sizeof(uint32_t), visitor, data);
            }
        }
        heap_free(blocks);
        heap_free(iblocks);
    }
    if (inode->blocks[14]){
        debug("Triple indirect block found\n");
        blocks = heap_alloc(fs->block_size);
        iblocks = heap_alloc(fs->block_size);
        i2blocks = heap_alloc(fs->block_size);
        ext2_device_gotoblock(fs, inode->blocks[12]);
        ext2_device_read_block(fs);
        memcpy(blocks,fs->block_buffer,fs->block_size);
        for (i=0;i<fs->block_size / sizeof(uint32_t);i++){
            if (blocks[i]) {
                ext2_device_gotoblock(fs, blocks[i]);
                ext2_device_read_block(fs);
                memcpy(iblocks,fs->block_buffer,fs->block_size);
                for (j=0;j<fs->block_size / sizeof(uint32_t);j++){
                    if (iblocks[j]) {
                        ext2_device_gotoblock(fs, iblocks[j]);
                        ext2_device_read_block(fs);
                        memcpy(i2blocks,fs->block_buffer,fs->block_size);
                        ext2_list_dir_blocks(fs, i2blocks, fs->block_size / sizeof(uint32_t), visitor, data);
                    }
                }
            }
        }
        heap_free(blocks);
        heap_free(iblocks);
        heap_free(i2blocks);
    }
}

static inline Ext2DirEntry* next_entry(Ext2DirEntry* entry){
    int offset = max(sizeof(Ext2DirEntry)-1,entry->rec_len);
    if (offset % 4){
        offset+=4-(offset % 4);
    }
    return (Ext2DirEntry*) ((void*)entry + offset);
}

static int16_t ext2_iter_dir_block(Ext2FileSystem* fs, uint32_t block_num, DirVisitor visitor, void* data){
    Ext2DirEntry* dir_entry;
    ext2_device_gotoblock(fs, block_num);
    ext2_device_read_block(fs);

    for(dir_entry = (Ext2DirEntry*)fs->block_buffer;
        (void*)dir_entry < ((void*)fs->block_buffer) + fs->block_size;//->file_type != EXT2_DIR_ENTRY_UNKNOWN;
        dir_entry = next_entry(dir_entry)){

        if (dir_entry->rec_len){
            if (visitor(fs, dir_entry, data)){
                return 1;
            }
        } 
    }
    return 0;
}
