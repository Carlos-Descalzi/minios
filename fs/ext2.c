#define NODEBUG
#include "fs/ext2.h"
#include "lib/stdint.h"
#include "lib/heap.h"
#include "lib/string.h"
#include "lib/stdlib.h"
#include "misc/debug.h"
#include "io/streams.h"

typedef struct {
    Ext2Inode inode;
    uint8_t in_use;
} InodeSlot;

typedef struct {
    FileSystem fs;
    Ext2Superblock super_block;
    uint32_t first_block_group_pos;
    uint32_t inodes_per_block;
    uint32_t group_descritors_per_block;
    uint32_t block_group_count;
    uint8_t* block_buffer;
    BlockPtr cache[5];
    uint8_t* block_cache;
    InodeSlot work_inodes[5];
} Ext2FileSystem;

typedef struct {
    Stream          stream;
    Ext2FileSystem* fs;
    Ext2Inode       inode; 
    uint8_t         mode;
    uint32_t        pos;
    uint32_t        numblocks;
    uint32_t        current_block;
    uint8_t*        block_buffer;
    //char            path[1];
} FileStream;

#define     FILE_STREAM(fs)  ((FileStream*)fs)

typedef int8_t (*DirVisitor)(Ext2FileSystem*, DirEntry*, void*);

#define E2FS(f)     ((Ext2FileSystem*)(f))
#define INODE(i)    ((Inode*)(i))

static const char FS_TYPE_NAME[] = "ext2";

static FileSystemType FS_TYPE;

#define OFFSET          (KIMAGESIZE)   // boot sector + kernel

static FileSystem*      create_fs               (FileSystemType* fs_type, BlockDevice* device);
static void             list_inodes             (FileSystem* fs, InodeVisitor visitor, void*data);
static void             close                   (FileSystem* fs);
static int32_t          load_inode              (FileSystem* fs, uint32_t inodenum, Inode* inode);
static uint32_t         find_inode              (FileSystem* fs, const char* path);
static int32_t          load                    (FileSystem* fs, Inode* inode, void* dest);
static uint32_t         read_block              (FileSystem* fs, Inode* inode, 
                                                uint32_t b_index, uint8_t* dest, uint32_t length);
static int32_t          get_direntry            (FileSystem* fs, Inode* inode, 
                                                uint32_t* offset, DirEntry* direntry);
static Inode*           alloc_inode             (FileSystem* fs);
static void             free_inode              (FileSystem* fs, Inode* inode);
static void             release_resources       (FileSystem* fs);
static Stream*          ext2_open_stream        (FileSystem* fs, uint32_t inodenum, uint32_t flags);
//static Stream*          ext2_stream_open        (FileSystem* fs, const char* path, uint32_t flags);
int16_t                 ext2_stream_read_byte   (Stream*);
int16_t                 ext2_stream_write_byte  (Stream*,uint8_t);
int16_t                 ext2_stream_read_bytes  (Stream*,uint8_t*,int16_t);
int16_t                 ext2_stream_write_bytes (Stream*,uint8_t*,int16_t);
uint32_t                ext2_stream_pos         (Stream*);
int16_t                 ext2_stream_seek        (Stream*,uint32_t);
uint32_t                ext2_stream_size        (Stream*);
void                    ext2_stream_close       (Stream*);
void                    ext2_stream_flush       (Stream*);




static char*            ext2_path_tokenize      (const char* path, char* buffer, uint8_t* pos);
static int16_t          ext2_iter_dir_block     (Ext2FileSystem* fs, uint32_t block_num, DirVisitor visitor, 
                                                void* func_data);
static void             ext2_list_dir_inode     (Ext2FileSystem* fs, Ext2Inode* inode, DirVisitor visitor, 
                                                void* func_data);
static uint32_t         find_next_inode         (Ext2FileSystem* fs, uint32_t inodenum, const char* dirent_name);
static DirEntry*        next_entry              (DirEntry* entry);
static int8_t           find_dirent             (Ext2FileSystem*fs, DirEntry* dirent, void* data);
static uint8_t*         cache_find              (Ext2FileSystem* fs, uint32_t disk_pos);
static uint8_t*         cache_take              (Ext2FileSystem* fs, uint32_t disk_pos);
static void             ext2_device_read_block  (Ext2FileSystem* fs); 
static void             ext2_device_read_block_b
                                                (Ext2FileSystem* fs, void* buffer, uint32_t length);
static uint32_t         get_block_by_index      (Ext2FileSystem* fs, Ext2Inode* inode, uint32_t block_index);

#define ext2_device_seek(fs, pos)               { block_device_seek(FILE_SYSTEM(fs)->device, (pos) + OFFSET); } 
#define ext2_device_gotoblock(fs, block)        { ext2_device_seek(fs, FILE_SYSTEM(fs)->block_size*(block)); }
#define ext2_device_pos(fs)                     (block_device_pos(FILE_SYSTEM(fs)->device) - OFFSET)

void ext2_register_type(void){
    FS_TYPE.type_name = FS_TYPE_NAME;
    FS_TYPE.create = create_fs;
    fs_register_type(&FS_TYPE);
}

FileSystem* create_fs(FileSystemType* fs_type, BlockDevice* device){
    int i;
    Ext2FileSystem* fs;
    debug("EXT2 - Opening ext2 filesystem\n");

    if (!device){
        debug("No device\n");
        return NULL;
    }

    if (DEVICE(device)->kind != DISK){
        debug("Invalid device type for ext2 fs\n");
        return NULL;
    }

    fs = heap_alloc(sizeof(Ext2FileSystem));

    if (!fs){
        debug("No memory\n");
        return NULL;
    }

    memset(fs,0,sizeof(Ext2FileSystem));

    block_device_seek(BLOCK_DEVICE(device), OFFSET + 1024); 
    block_device_read(BLOCK_DEVICE(device), (uint8_t*)&(fs->super_block), sizeof(Ext2Superblock));

    if (fs->super_block.magic != 0xef53){
        debug("EXT2 - Bad magic number ("); 
        debug_i(fs->super_block.magic,16); 
        debug(") - no ext2 filesyste found\n");
        heap_free(fs);
        return NULL;
    } else {
        debug("EXT2 - Magic number found\n");
    }

    FILE_SYSTEM(fs)->type = fs_type;
    FILE_SYSTEM(fs)->device = device;
    FILE_SYSTEM(fs)->list_inodes = list_inodes;
    FILE_SYSTEM(fs)->close = close;
    FILE_SYSTEM(fs)->find_inode = find_inode;
    FILE_SYSTEM(fs)->load_inode = load_inode;
    FILE_SYSTEM(fs)->load = load;
    FILE_SYSTEM(fs)->read_block = read_block;
    FILE_SYSTEM(fs)->get_direntry = get_direntry;
    FILE_SYSTEM(fs)->alloc_inode = alloc_inode;
    FILE_SYSTEM(fs)->free_inode = free_inode;
    FILE_SYSTEM(fs)->open_stream = ext2_open_stream;
    //FILE_SYSTEM(fs)->stream_open = ext2_stream_open;
    FILE_SYSTEM(fs)->release_resources = release_resources;
    FILE_SYSTEM(fs)->inode_size = sizeof(Ext2Inode);
    FILE_SYSTEM(fs)->block_size = 1024 << fs->super_block.log_block_size;


    debug("Block size:");debug_i(FILE_SYSTEM(fs)->block_size,10);debug("\n");

    fs->block_buffer = NULL;
    fs->first_block_group_pos = FILE_SYSTEM(fs)->block_size 
        * (FILE_SYSTEM(fs)->block_size == 1024 ? 2 : 1);
    fs->inodes_per_block = FILE_SYSTEM(fs)->block_size / sizeof(Ext2Inode);
    fs->group_descritors_per_block = FILE_SYSTEM(fs)->block_size / sizeof(Ext2BlockGroupDescriptor);
    fs->block_group_count = fs->super_block.block_count / fs->super_block.blocks_per_group;

    // Allocate space for 5 blocks 

    fs->block_cache = heap_alloc(FILE_SYSTEM(fs)->block_size * 5);

    for (i=0;i<5;i++){
        fs->cache[i].disk_pos = 0;
        fs->cache[i].pos = FILE_SYSTEM(fs)->block_size * i;
    }

    return FILE_SYSTEM(fs);
}

#define swap(a,b,t) { t = a; a = b; b = t;}

static uint8_t* cache_find(Ext2FileSystem* fs, uint32_t disk_pos){
    int i;
    for (i=0;i<5;i++){
        if (fs->cache[i].disk_pos == disk_pos){
            uint8_t* pos = fs->block_cache + fs->cache[i].pos;
            if (i > 0){
                uint32_t tmp;
                // Raise in cache
                swap(fs->cache[i-1].disk_pos ,fs->cache[i].disk_pos,tmp);
                swap(fs->cache[i-1].pos,fs->cache[i].pos,tmp);
            }
            return pos;
        }
    }
    return NULL;
}
#define to_disk_pos(fs,bn)  ((bn*fs->block_size)+OFFSET)

static uint8_t* cache_take(Ext2FileSystem* fs, uint32_t disk_pos){
    int i;
    for (i=0;i<5;i++){
        if (!fs->cache[i].disk_pos){
            fs->cache[i].disk_pos = disk_pos;
            return fs->block_cache+fs->cache[i].pos;
        }
    }
    // otherwise last recently used.
    fs->cache[4].disk_pos = disk_pos;
    return fs->block_cache + fs->cache[4].pos;
}


static void ext2_device_read_block(Ext2FileSystem* fs){ 
    uint32_t disk_pos = block_device_pos(FILE_SYSTEM(fs)->device);
    uint8_t *block_buffer = cache_find(fs, disk_pos);

    if (block_buffer){
        fs->block_buffer = block_buffer;
    } else {
        fs->block_buffer = cache_take(fs, disk_pos);
        block_device_read(FILE_SYSTEM(fs)->device, fs->block_buffer, FILE_SYSTEM(fs)->block_size); 
    }
}

static void ext2_device_read_block_b(Ext2FileSystem* fs, void* buffer, uint32_t length){
    uint32_t disk_pos = block_device_pos(FILE_SYSTEM(fs)->device);
    uint8_t *block_buffer = cache_find(fs, disk_pos);

    if (!length){
        length = FILE_SYSTEM(fs)->block_size;
    }

    if(block_buffer){
        debug("EXT2 - Cache hit for ");debug_i(disk_pos,10);debug("\n");
    } else {
        debug("EXT2 - Cache miss for ");debug_i(disk_pos,10);debug("\n");
        block_buffer = cache_take(fs, disk_pos);
        block_device_read(FILE_SYSTEM(fs)->device, block_buffer, FILE_SYSTEM(fs)->block_size); 
    }
    memcpy(buffer, block_buffer, length);
}


static void do_list_inodes(Ext2FileSystem* fs, InodeVisitor visitor, void*data,
                            Ext2BlockGroupDescriptor* descriptors,
                            Ext2Inode* inode_table, 
                            uint32_t *ngroups,
                            uint32_t *inodenum){
    int i, j;
    uint32_t pos;
    uint32_t ninodetableblocks;

    ninodetableblocks = fs->super_block.blocks_per_group / fs->inodes_per_block;

    for (i=0;i< fs->group_descritors_per_block && *ngroups > 0;i++,(*ngroups)--){

        if (descriptors[i].inode_table == 0){
            return;
        }

        ext2_device_gotoblock(fs, descriptors[i].inode_table);

        for(; ninodetableblocks > 0;ninodetableblocks--){

            ext2_device_read_block_b(fs,inode_table,0);
            pos = ext2_device_pos(fs);

            for (j=0;j<fs->inodes_per_block;j++,(*inodenum)++){
                if (visitor(FILE_SYSTEM(fs), *inodenum, INODE(&(inode_table[j])),data)){
                    return;
                }
            }
            ext2_device_seek(fs, pos);
        }
    }
}

static void list_inodes(FileSystem* fs, InodeVisitor visitor, void*data){

    Ext2BlockGroupDescriptor* descriptors;
    Ext2Inode* inode_table;
    uint32_t pos;
    uint32_t ngroups;
    uint32_t inodenum = 1;

    descriptors = heap_alloc(fs->block_size);
    inode_table = heap_alloc(fs->block_size);
    ngroups = E2FS(fs)->block_group_count;
    pos = E2FS(fs)->first_block_group_pos;

    while(ngroups > 0){

        ext2_device_seek(E2FS(fs), pos);
        ext2_device_read_block_b(E2FS(fs), descriptors,0);
        pos = ext2_device_pos(E2FS(fs));
        do_list_inodes(
            E2FS(fs), 
            visitor, 
            data, 
            descriptors, 
            inode_table, 
            &ngroups, 
            &inodenum
        );
    }

    heap_free(inode_table);
    heap_free(descriptors);
}

static void close(FileSystem* fs){
    heap_free(E2FS(fs)->block_cache);
    heap_free(fs);
}

static int32_t load_inode(FileSystem* fs, uint32_t inodenum, Inode* inode){
    uint32_t block_group = (inodenum - 1) / E2FS(fs)->super_block.inodes_per_group;
    uint32_t index = (inodenum - 1) % E2FS(fs)->super_block.inodes_per_group;
    uint32_t block_group_table_block = (block_group / E2FS(fs)->group_descritors_per_block) * fs->block_size;
    uint32_t block_group_table_offset = block_group % E2FS(fs)->group_descritors_per_block;
    uint32_t inode_table;

    Ext2Inode* table;

    ext2_device_seek(E2FS(fs), 
        E2FS(fs)->first_block_group_pos 
        + block_group_table_block 
        * fs->block_size
    );

    ext2_device_read_block(E2FS(fs));
    inode_table = ((Ext2BlockGroupDescriptor*)E2FS(fs)->block_buffer)[block_group_table_offset].inode_table;
    inode_table += (index / E2FS(fs)->inodes_per_block);

    ext2_device_gotoblock(E2FS(fs),inode_table);
    index = index % E2FS(fs)->inodes_per_block;

    ext2_device_read_block(E2FS(fs));

    table = (Ext2Inode*)E2FS(fs)->block_buffer;
    memcpy(inode,&(table[index]),sizeof(Ext2Inode));

    return 0;
}

static uint32_t find_inode(FileSystem* fs, const char* path){
    char path_token[256];
    uint8_t pos=0;
    uint32_t current_inode = EXT2_INODE_ROOT_DIR; 

    if (!strcmp(path,"/")){
        return current_inode;
    }

    memset(path_token,0,sizeof(path_token));
    while(ext2_path_tokenize(path,path_token,&pos) && current_inode){
        if (!strcmp(path_token,"/")){
            current_inode = 2;
        } else {
            current_inode = find_next_inode(E2FS(fs), current_inode, path_token);
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

static int32_t load(FileSystem* fs, Inode* inode, void* dest){
    uint32_t size = inode->size;
    uint32_t offset = 0;
    uint32_t to_read;
    uint32_t block_index = 0;
    uint32_t block_num;

    while(size){
        to_read = min(size, fs->block_size);
        block_num = get_block_by_index(E2FS(fs), E2INODE(inode), block_index);
        ext2_device_gotoblock(E2FS(fs), block_num);
        ext2_device_read_block_b(E2FS(fs), dest + offset, to_read);
        debug(dest + offset);
        offset+=to_read;
        size-=to_read;
        block_index++;
    }
    
    return offset;
}

static uint32_t get_block_by_index(Ext2FileSystem* fs, Ext2Inode* inode, uint32_t block_index){
    uint32_t entries_per_block = FILE_SYSTEM(fs)->block_size / sizeof(uint32_t);

    debug("EXT2 - get_block_by_index:");debug_i(block_index,10);debug("\n");

    if (block_index < 12){
        return E2INODE(inode)->blocks[block_index];
    } else if (block_index >= 12 && block_index < entries_per_block + 12){
        uint32_t iblock = E2INODE(inode)->blocks[12];
        debug("\tIndirect block:");debug_i(iblock,10);debug("\n");
        ext2_device_gotoblock(E2FS(fs), iblock);
        ext2_device_read_block(E2FS(fs));
        block_index-=12;
        uint32_t bnum = ((uint32_t*)E2FS(fs)->block_buffer)[block_index];
        debug("\tIndirect block number:");debug_i(bnum,10);debug("\n");
        return bnum;

    } else if (block_index >= entries_per_block + 12){
        // TODO Terminar
    }
    return 0;
}

static uint32_t read_block(FileSystem* fs, Inode* inode, 
                         uint32_t b_index, uint8_t* dest, uint32_t length){
    uint32_t block;

    block = get_block_by_index(E2FS(fs), E2INODE(inode), b_index);
    ext2_device_gotoblock(E2FS(fs), block);
    ext2_device_read_block_b(E2FS(fs), dest, length);

    return 0;
}

static int8_t find_dirent(Ext2FileSystem*fs, DirEntry* dirent, void* data){
    DirentFindData* find_data = data;

    if (dirent->name_len == find_data->namelen 
        && !strncmp(dirent->name, find_data->dirent_name,find_data->namelen)){
        find_data->inode = dirent->inode;
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

    if(load_inode(FILE_SYSTEM(fs), inodenum, INODE(&inode))){
        debug("EXT2 - find_next_inode - error\n");
        return 0;
    }
    if (inode.inode.type != EXT2_INODE_TYPE_DIRECTORY){
        debug("\tNot a directory:");debug(dirent_name);debug("\n");
        return 0;
    } else {
        debug("\tIs a directory:");debug(dirent_name);debug("\n");
    }

    ext2_list_dir_inode(fs, &inode, find_dirent, &dirent_find_data);

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
static int32_t get_direntry(FileSystem* fs, Inode* inode, uint32_t* offset, DirEntry* direntry){

    if (inode->type != EXT2_INODE_TYPE_DIRECTORY){
        debug("Not a directory\n");
        return -1;
    }

    uint32_t block = *offset / fs->block_size;
    uint32_t block_offset = *offset % fs->block_size;

    uint32_t block_pos = get_block_by_index(E2FS(fs), E2INODE(inode), block);
    ext2_device_gotoblock(E2FS(fs), block_pos);
    ext2_device_read_block(E2FS(fs));

    DirEntry* entry = (DirEntry*)(E2FS(fs)->block_buffer + block_offset);
    memcpy(direntry, entry, sizeof(DirEntry) + entry->name_len);
    *offset+=entry->rec_len;

    return *offset == fs->block_size ? 1 : 0;
}

static void ext2_list_dir_inode(Ext2FileSystem* fs, Ext2Inode* inode, DirVisitor visitor, void* data){
    uint32_t *blocks;
    uint32_t *iblocks;
    uint32_t *i2blocks;
    int i,j;
    int finish;

    if (ext2_list_dir_blocks(fs,inode->blocks,12, visitor, data)){
        return;
    }
    if (inode->blocks[12]){
        blocks = heap_alloc(FILE_SYSTEM(fs)->block_size);
        ext2_device_gotoblock(fs, inode->blocks[12]);
        ext2_device_read_block_b(fs, blocks,0);
        finish = ext2_list_dir_blocks(
            fs, 
            blocks, 
            FILE_SYSTEM(fs)->block_size / sizeof(uint32_t), 
            visitor, 
            data
        );
        heap_free(blocks);
    }
    if (finish){
        return;
    }
    if (inode->blocks[13]){
        blocks = heap_alloc(FILE_SYSTEM(fs)->block_size);
        iblocks = heap_alloc(FILE_SYSTEM(fs)->block_size);
        ext2_device_gotoblock(fs, inode->blocks[12]);
        ext2_device_read_block_b(fs,blocks,0);
        for (i=0;!finish && i<FILE_SYSTEM(fs)->block_size / sizeof(uint32_t);i++){
            if (blocks[i]) {
                ext2_device_gotoblock(fs, blocks[i]);
                ext2_device_read_block_b(fs,iblocks,0);
                finish = ext2_list_dir_blocks(
                    fs, 
                    iblocks, 
                    FILE_SYSTEM(fs)->block_size / sizeof(uint32_t), 
                    visitor, 
                    data
                );
            }
        }
        heap_free(blocks);
        heap_free(iblocks);
    }
    if (finish){
        return;
    }
    if (inode->blocks[14]){
        blocks = heap_alloc(FILE_SYSTEM(fs)->block_size);
        iblocks = heap_alloc(FILE_SYSTEM(fs)->block_size);
        i2blocks = heap_alloc(FILE_SYSTEM(fs)->block_size);
        ext2_device_gotoblock(fs, inode->blocks[12]);
        ext2_device_read_block_b(fs,blocks,0);
        for (i=0;!finish && i<FILE_SYSTEM(fs)->block_size / sizeof(uint32_t);i++){
            if (blocks[i]) {
                ext2_device_gotoblock(fs, blocks[i]);
                ext2_device_read_block_b(fs,iblocks,0);
                for (j=0;!finish && j<FILE_SYSTEM(fs)->block_size / sizeof(uint32_t);j++){
                    if (iblocks[j]) {
                        ext2_device_gotoblock(fs, iblocks[j]);
                        ext2_device_read_block_b(fs,i2blocks,0);
                        finish = ext2_list_dir_blocks(
                            fs, 
                            i2blocks, 
                            FILE_SYSTEM(fs)->block_size / sizeof(uint32_t), 
                            visitor, 
                            data
                        );
                    }
                }
            }
        }
        heap_free(blocks);
        heap_free(iblocks);
        heap_free(i2blocks);
    }
}

static inline DirEntry* next_entry(DirEntry* entry){
    int offset = max(sizeof(DirEntry)-1,entry->rec_len);
    if (offset % 4){
        offset+=4-(offset % 4);
    }
    return (DirEntry*) ((void*)entry + offset);
}

static int16_t ext2_iter_dir_block(Ext2FileSystem* fs, uint32_t block_num, DirVisitor visitor, void* data){
    DirEntry* dir_entry;
    ext2_device_gotoblock(fs, block_num);
    ext2_device_read_block(fs);

    for(dir_entry = (DirEntry*)fs->block_buffer;
        (void*)dir_entry < ((void*)fs->block_buffer) + FILE_SYSTEM(fs)->block_size;
        dir_entry = next_entry(dir_entry)){

        if (dir_entry->rec_len){
            if (visitor(fs, dir_entry, data)){
                return 1;
            }
        } 
    }
    return 0;
}

//Stream* ext2_stream_open(FileSystem* fs, const char* path, uint32_t flags){
static Stream* ext2_open_stream (FileSystem* fs, uint32_t inodenum, uint32_t flags){
    //uint32_t inodenum;
    FileStream* stream;

    debug("EXT2 - Find inode for ");debug(path);debug("\n");
    //inodenum = find_inode(fs, path);
    debug("EXT2 - Inode: ");debug_i(inodenum,10);debug("\n");

    if (inodenum){
        stream = heap_alloc(sizeof(FileStream));// + strlen(path));
        if (!stream){
            debug("EXT2 - No memory for creating stream\n");
            return NULL;
        }
        memset(stream,0,sizeof(FileStream));
        stream->fs = E2FS(fs);
        stream->block_buffer = heap_alloc(fs->block_size);
        load_inode(fs, inodenum, INODE(&(stream->inode)));
        //strcpy(stream->path, path);
        debug("EXT2 - inode size:");debug_i(fs->block_size,10);debug("\n");
        stream->numblocks = stream->inode.inode.size / fs->block_size;
        if (stream->inode.inode.size % fs->block_size){
            stream->numblocks++;
        }
        STREAM(stream)->async = 0;
        STREAM(stream)->seekable = 1;
        STREAM(stream)->nonblocking = (flags & O_NONBLOCK) != 0;
        STREAM(stream)->readable = (flags & O_RDONLY) != 0;
        STREAM(stream)->writeable = (flags & O_WRONLY) != 0;
        STREAM(stream)->read_byte = ext2_stream_read_byte;
        STREAM(stream)->write_byte = ext2_stream_write_byte;
        STREAM(stream)->read_bytes = ext2_stream_read_bytes;
        STREAM(stream)->write_bytes = ext2_stream_write_bytes;
        STREAM(stream)->flush = ext2_stream_flush;
        STREAM(stream)->pos = ext2_stream_pos;
        STREAM(stream)->seek = ext2_stream_seek;
        STREAM(stream)->size = ext2_stream_size;
        STREAM(stream)->close = ext2_stream_close;
        stream->current_block = 0;
        debug("EXT2 - reading blocks\n");
        read_block(fs, INODE(&(stream->inode)), stream->current_block, stream->block_buffer, 0);
        return STREAM(stream);
    }
    return NULL;
}
void ext2_stream_close(Stream* stream){
    heap_free(stream);
}

int16_t ext2_stream_read_byte(Stream* stream){
    uint32_t rel_pos;
    uint32_t block_size;
    uint32_t block;
    uint8_t val;

    if (FILE_STREAM(stream)->pos >= FILE_STREAM(stream)->inode.inode.size){
        return -1;
    }

    FileSystem* fs = FILE_SYSTEM(FILE_STREAM(stream)->fs);

    block_size = fs->block_size;
    rel_pos = FILE_STREAM(stream)->pos % block_size;
    val = FILE_STREAM(stream)->block_buffer[rel_pos];

    FILE_STREAM(stream)->pos++;

    if (!(FILE_STREAM(stream)->pos % fs->block_size)){
        FILE_STREAM(stream)->current_block++;
        block = FILE_STREAM(stream)->pos / fs->block_size;
        read_block(
            FILE_SYSTEM(FILE_STREAM(stream)->fs),  
            INODE(&(FILE_STREAM(stream)->inode)),
            FILE_STREAM(stream)->current_block,
            FILE_STREAM(stream)->block_buffer,
            0
        );
    }

    return val;
}
int16_t ext2_stream_write_byte(Stream* stream,uint8_t byte){
    if (stream_writeable(stream)){
        // TODO finish
        return 0;
    }
    return -1;
}
int16_t ext2_stream_write_bytes(Stream* stream,uint8_t* bytes,int16_t size){
    if (stream_writeable(stream)){
        // TODO finish
        return 0;
    }
    return -1;
}
int16_t ext2_stream_read_bytes(Stream* stream,uint8_t* bytes,int16_t size){

    if (stream_readable(stream)){
        uint32_t block_size;
        uint32_t offset;
        uint32_t block;
        uint16_t nblocks;
        uint32_t to_read;
        uint16_t i;
        uint32_t bytes_read;

        if (FILE_STREAM(stream)->pos >= FILE_STREAM(stream)->inode.inode.size){
            return 0;
        }
        block_size = FILE_SYSTEM(FILE_STREAM(stream)->fs)->block_size;
        offset = FILE_STREAM(stream)->pos % block_size;
        block = FILE_STREAM(stream)->pos / block_size;
        nblocks = size / block_size + (size % block_size ? 1 : 0);
        bytes_read = 0;

        debug("EXT2 - Block:");debug_i(block,10);debug("\n");

        for (i=0;i<nblocks;i++){
            read_block(
                FILE_SYSTEM(FILE_STREAM(stream)->fs),
                INODE(&(FILE_STREAM(stream)->inode)),
                block + i,
                FILE_STREAM(stream)->block_buffer,
                FILE_SYSTEM(FILE_STREAM(stream)->fs)->block_size
            );
            to_read = min(size, block_size - offset);
            memcpy(bytes + bytes_read, FILE_STREAM(stream)->block_buffer + offset, to_read);
            offset = 0;
            size -= to_read;
            bytes_read += to_read;
        }
        FILE_STREAM(stream)->pos += bytes_read;

        return bytes_read;
    }
    return -1;
}
uint32_t ext2_stream_pos(Stream* stream){
    return FILE_STREAM(stream)->pos;
}
int16_t ext2_stream_seek(Stream* stream,uint32_t pos){
    debug("EXT2 - seek ");debug_i(pos,10);debug("\n");
    if (pos > FILE_STREAM(stream)->inode.inode.size -1){
        return -1;
    }
    FILE_STREAM(stream)->pos = pos;
    return 0;
}
uint32_t ext2_stream_size(Stream* stream){
    return FILE_STREAM(stream)->inode.inode.size;
}

static Inode* alloc_inode(FileSystem* fs){
    for (int i=0;i<5;i++){
        if (!E2FS(fs)->work_inodes[i].in_use){
            E2FS(fs)->work_inodes[i].in_use = 1;
            debug("EXT2 - alloc_inode: node allocated\n");
            return INODE(&(E2FS(fs)->work_inodes[i].inode));
        }
    }
    debug("EXT2 - alloc_inode: No more inodes\n");
    return NULL;
}
static void free_inode(FileSystem* fs, Inode* inode){
    for (int i=0;i<5;i++){
        if (&(E2FS(fs)->work_inodes[i].inode) == inode){
            E2FS(fs)->work_inodes[i].in_use = 0;
            break;
        }
    }
}
static void release_resources(FileSystem* fs){
    for (int i=0;i<5;i++){
        if (E2FS(fs)->work_inodes[i].in_use){
            E2FS(fs)->work_inodes[i].in_use = 0;
        }
    }
}
void ext2_stream_flush(Stream* stream){
}
