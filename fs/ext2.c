#include "fs/ext2.h"
#include "lib/stdint.h"
#include "lib/heap.h"
#include "lib/string.h"
#include "lib/stdlib.h"
#include "misc/debug.h"


#define OFFSET      (512 + 65536)   // boot sector + kernel

static char*            ext2_path_tokenize  (const char* path, char* buffer, uint8_t* pos);
static int16_t          ext2_iter_dir_block (Ext2FileSystem* fs, uint32_t block_num, DirVisitor visitor, 
                                            void* func_data);
static void             ext2_list_dir_inode (Ext2FileSystem* fs, Ext2Inode* inode, DirVisitor visitor, 
                                            void* func_data);
static uint32_t         find_next_inode     (Ext2FileSystem* fs, uint32_t inodenum, const char* dirent_name);
static Ext2DirEntry*    next_entry          (Ext2DirEntry* entry);
//static void show_inode(uint32_t number, Ext2Inode* inode);
//static void show_dirent(Ext2DirEntry*entry);

#define ext2_device_seek(fs, pos)           { block_device_seek(fs->device, (pos) + OFFSET); } 
#define ext2_device_read_block(fs)          { block_device_read(fs->device, fs->block_buffer, fs->block_size); }
#define ext2_device_gotoblock(fs, block)    { ext2_device_seek(fs, fs->block_size*(block)); }
#define ext2_device_pos(fs)                 (block_device_pos(fs->device) - OFFSET)

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
    fs->inodes_per_block = fs->block_size / sizeof(Ext2Inode);
    fs->group_descritors_per_block = fs->block_size / sizeof(Ext2BlockGroupDescriptor);
    fs->block_group_count = fs->super_block.block_count / fs->super_block.blocks_per_group;

    debug("EXT2 - Ext2 Block size: "); debug_i(fs->block_size,10); debug("\n");
    debug("EXT2 - Inode size:"); debug_i(fs->super_block.inode_size,10); debug("\n");
    debug("EXT2 - Inode count:"); debug_i(fs->super_block.inode_count,10); debug("\n");
    debug("EXT2 - Block count:"); debug_i(fs->super_block.block_count,10); debug("\n");
    debug("EXT2 - Blocks per group:"); debug_i(fs->super_block.blocks_per_group, 10); debug("\n");
    debug("EXT2 - Free Inode count:"); debug_i(fs->super_block.free_inode_count,10); debug("\n");
    debug("EXT2 - Free Block count:"); debug_i(fs->super_block.free_block_count,10); debug("\n");
    return fs;
}

static void do_list_inodes(Ext2FileSystem* fs, InodeVisitor visitor, void*data,
                            Ext2BlockGroupDescriptor* descriptors,
                            Ext2Inode* inode_table, 
                            uint32_t *ngroups,
                            uint32_t *inodenum){
    int i, j;
    uint32_t pos;
    uint32_t ninodetableblocks = fs->super_block.blocks_per_group / fs->inodes_per_block;

    for (i=0;i< fs->group_descritors_per_block && *ngroups > 0;i++,(*ngroups)--){
        if (descriptors[i].inode_table == 0){
            return;
        }

       // debug("EXT2 - Block ");debug_i(i,10);debug(", inode table ");
        //debug_i(descriptors[i].inode_table,10); 
        //debug(", n dirs:");debug_i(descriptors[i].used_dirs_count,10);
        //debug(", used blocks:");debug_i(fs->super_block.blocks_per_group - descriptors[i].free_blocks_count,10);
        //debug(", free blocks:");debug_i(descriptors[i].free_blocks_count,10);debug("\n");

        ext2_device_gotoblock(fs, descriptors[i].inode_table);
        for(; ninodetableblocks > 0;ninodetableblocks--){
            ext2_device_read_block(fs);
            pos = ext2_device_pos(fs);
            memcpy(inode_table,fs->block_buffer,fs->block_size);

            for (j=0;j<fs->inodes_per_block;j++,(*inodenum)++){
                if (visitor(fs, *inodenum, &(inode_table[j]),data)){
                    return;
                }
            }
            ext2_device_seek(fs, pos);
        }
    }
}

void ext2_list_inodes(Ext2FileSystem* fs, InodeVisitor visitor, void*data){
    uint32_t pos;
    Ext2BlockGroupDescriptor* descriptors = heap_alloc(fs->block_size);
    Ext2Inode* inode_table = heap_alloc(fs->block_size);
    uint32_t ngroups = fs->block_group_count;
    uint32_t inodenum = 1;

    //debug("EXT2 - Number of block groups:"); debug_i(ngroups, 10); debug("\n");
    //debug("EXT2 - Number of inodes per block:"); debug_i(fs->inodes_per_block,10); debug("\n");

    pos = fs->first_block_group_pos;

    while(ngroups > 0){
        ext2_device_seek(fs, pos);
        ext2_device_read_block(fs);
        pos = ext2_device_pos(fs);
        memcpy(descriptors,fs->block_buffer,fs->block_size);

        do_list_inodes(fs, visitor, data, descriptors, inode_table, &ngroups, &inodenum);
    }
    heap_free(inode_table);
    heap_free(descriptors);
}
void ext2_list_directory (Ext2FileSystem* fs, Ext2Inode* inode, DirVisitor visitor, void* data){
    ext2_list_dir_inode(fs, inode, visitor, data);
}

void ext2_close(Ext2FileSystem* fs){
    heap_free(fs->block_buffer);
    heap_free(fs);
}

int32_t ext2_load_inode(Ext2FileSystem* fs, uint32_t inodenum, Ext2Inode* inode){
    uint32_t block_group = (inodenum - 1) / fs->super_block.inodes_per_group;
    uint32_t index = (inodenum - 1) % fs->super_block.inodes_per_group;
    uint32_t block_group_table_block = (block_group / fs->group_descritors_per_block) * fs->block_size;
    uint32_t block_group_table_offset = block_group % fs->group_descritors_per_block;
    uint32_t inode_table;
    Ext2Inode* table;

    //debug("EXT2 - ext2_load_inode\n");

    //debug("EXT2 - Load inode "); debug_i(inodenum, 10);
    //debug(", block group:"); debug_i(block_group,10);
    //debug(", offset:"); debug_i(block_group_table_offset,10);
    //debug(", index:"); debug_i(index,10);
    //debug("\n");

    ext2_device_seek(fs, fs->first_block_group_pos + block_group_table_block * fs->block_size);

    ext2_device_read_block(fs);
    inode_table = ((Ext2BlockGroupDescriptor*)fs->block_buffer)[block_group_table_offset].inode_table;
    //debug("EXT2 Inode table at block: ");debug_i(inode_table,10);debug("\n");

    //debug("Inodes per block:");debug_i(fs->inodes_per_block,10);debug("\n");
    //debug("Offset on table:");debug_i(index / fs->inodes_per_block,10);debug("\n");
    inode_table += (index / fs->inodes_per_block);
    //debug("New inode table block:");debug_i(inode_table,10);debug("\n");

    ext2_device_gotoblock(fs,inode_table);
    index = index % fs->inodes_per_block;
    //debug("Adjusted index:");debug_i(index,10);debug("\n");

    ext2_device_read_block(fs);

    table = (Ext2Inode*)fs->block_buffer;
    memcpy(inode,&(table[index]),sizeof(Ext2Inode));

    return 0;
}

uint32_t ext2_find_inode(Ext2FileSystem* fs, const char* path){
    char path_token[256];
    uint8_t pos=0;
    uint32_t current_inode = 2; // root directory inode
    memset(path_token,0,sizeof(path_token));
    while(ext2_path_tokenize(path,path_token,&pos) && current_inode){
        //debug("Token:"); debug(path_token); debug("\n");
        if (!strcmp(path_token,"/")){
            //debug("Set root\n");
            current_inode = 2;
        } else {
            current_inode = find_next_inode(fs, current_inode, path_token);
        }
        memset(path_token,0,sizeof(path_token));
    }
    if (!current_inode){
        debug("Not found\n");
    }

    return current_inode;
}

typedef struct {
    const char* dirent_name;
    uint16_t namelen;
    uint32_t inode;
} DirentFindData;

static int8_t find_dirent(Ext2FileSystem*fs, Ext2DirEntry* dirent, void* data){
    DirentFindData* find_data = data;
    //debug("EXT2 - find_dirent\n");

    if (dirent->name_len == find_data->namelen 
        && !strncmp(dirent->name, find_data->dirent_name,find_data->namelen)){
        find_data->inode = dirent->inode;
        //debug("EXT2 - Found:");
        //debug(find_data->dirent_name);debug(":");
        //debug_i(find_data->inode,10);debug("\n");
        return 1;
    } 
    return 0;
}

static uint32_t find_next_inode(Ext2FileSystem* fs, uint32_t inodenum, const char* dirent_name){
    DirentFindData dirent_find_data = {
        .dirent_name = dirent_name, 
        .namelen = strlen(dirent_name),
        .inode=0 
    };
    Ext2Inode inode;
    //debug("EXT2 - find_next_inode:");debug_i(inodenum,10);debug(",");debug(dirent_name);debug("\n");
    if(ext2_load_inode(fs, inodenum, &inode)){
        return 0;
    }
    if (inode.mode >> 12 != 4){
        debug("Not a directory\n");
        return 0;
    }
    //show_inode(inodenum, &inode);
    ext2_list_dir_inode(fs,&inode, find_dirent, &dirent_find_data);
    return dirent_find_data.inode;
}

static char* ext2_path_tokenize(const char* path, char* buffer, uint8_t* pos){

    if (path[*pos]){
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
            if (path[*pos] == '/'){
                (*pos)++;
            }
            buffer[i] = '\0';
            return buffer;
        }
    }
    return NULL;
}

static int ext2_list_dir_blocks(Ext2FileSystem* fs, uint32_t* blocks, uint32_t nblocks, DirVisitor visitor, void* data){
    int i;
    //debug("EXT2 - ext2_list_dir_blocks\n");
    for (i=0;i<nblocks;i++){
        if (blocks[i] != 0){
            debug("EXT2 - Listing block "); debug_i(blocks[i],10);debug("\n");
            if (ext2_iter_dir_block(fs, blocks[i], visitor, data)){
                return 1;
            }
        }
    }
    return 0;
}

static void ext2_list_dir_inode(Ext2FileSystem* fs, Ext2Inode* inode, DirVisitor visitor, void* data){
    uint32_t *blocks;
    uint32_t *iblocks;
    uint32_t *i2blocks;
    int i,j;
    int finish;
    //debug("EXT2 - ext2_list_dir_inode\n");
    //debug("EXT2 - Direct blocks\n");
    if (ext2_list_dir_blocks(fs,inode->blocks,12, visitor, data)){
        return;
    }
    if (inode->blocks[12]){
        //debug("Simply indirect block found\n");
        blocks = heap_alloc(fs->block_size);
        ext2_device_gotoblock(fs, inode->blocks[12]);
        ext2_device_read_block(fs);
        memcpy(blocks,fs->block_buffer,fs->block_size);
        finish = ext2_list_dir_blocks(fs, blocks, fs->block_size / sizeof(uint32_t), visitor, data);
        heap_free(blocks);
    }
    if (finish){
        return;
    }
    if (inode->blocks[13]){
        //debug("Double indirect block found\n");
        blocks = heap_alloc(fs->block_size);
        iblocks = heap_alloc(fs->block_size);
        ext2_device_gotoblock(fs, inode->blocks[12]);
        ext2_device_read_block(fs);
        memcpy(blocks,fs->block_buffer,fs->block_size);
        for (i=0;!finish && i<fs->block_size / sizeof(uint32_t);i++){
            if (blocks[i]) {
                ext2_device_gotoblock(fs, blocks[i]);
                ext2_device_read_block(fs);
                memcpy(iblocks,fs->block_buffer,fs->block_size);
                finish = ext2_list_dir_blocks(fs, iblocks, fs->block_size / sizeof(uint32_t), visitor, data);
            }
        }
        heap_free(blocks);
        heap_free(iblocks);
    }
    if (finish){
        return;
    }
    if (inode->blocks[14]){
        //debug("Triple indirect block found\n");
        blocks = heap_alloc(fs->block_size);
        iblocks = heap_alloc(fs->block_size);
        i2blocks = heap_alloc(fs->block_size);
        ext2_device_gotoblock(fs, inode->blocks[12]);
        ext2_device_read_block(fs);
        memcpy(blocks,fs->block_buffer,fs->block_size);
        for (i=0;!finish && i<fs->block_size / sizeof(uint32_t);i++){
            if (blocks[i]) {
                ext2_device_gotoblock(fs, blocks[i]);
                ext2_device_read_block(fs);
                memcpy(iblocks,fs->block_buffer,fs->block_size);
                for (j=0;!finish && j<fs->block_size / sizeof(uint32_t);j++){
                    if (iblocks[j]) {
                        ext2_device_gotoblock(fs, iblocks[j]);
                        ext2_device_read_block(fs);
                        memcpy(i2blocks,fs->block_buffer,fs->block_size);
                        finish = ext2_list_dir_blocks(fs, i2blocks, fs->block_size / sizeof(uint32_t), visitor, data);
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
    //debug("EXT2 - ext2_iter_dir_block ");
    //debug_i(block_num,10);
    //debug("\n");

    for(dir_entry = (Ext2DirEntry*)fs->block_buffer;
        (void*)dir_entry < ((void*)fs->block_buffer) + fs->block_size;
        dir_entry = next_entry(dir_entry)){
        //show_dirent(dir_entry);
        if (dir_entry->rec_len){
            if (visitor(fs, dir_entry, data)){
                return 1;
            }
        } 
    }
    return 0;
}

/*
static void show_dirent(Ext2DirEntry*entry){
    debug("Show dir entry:");
    debug("\tI-node:");debug_i(entry->inode,10);
    debug(",Length:");debug_i(entry->rec_len,10);
    debug(",Name length:");debug_i(entry->name_len,10);
    debug(",Name:");debug_s(entry->name,entry->name_len);debug("\n");
}

static void show_inode(uint32_t number, Ext2Inode* inode){
    int i;
    debug("Show inode number:");debug_i(number,10);debug("\n");
    debug("\tType:");debug_i(inode->mode >> 12,16);debug("\n");
    debug("\tMode:");debug_i(inode->mode & 0xfff,8);debug("\n");
    debug("\tlink count:");debug_i(inode->link_count,10);debug("\n");
    debug("\tsector count:");debug_i(inode->block_count,10);debug("\n");
    for (i=0;i<15;i++){
        debug("\tblock ");debug_i(i,10);debug(":");debug_i(inode->blocks[i],10);debug("\n");
    }
}
*/
