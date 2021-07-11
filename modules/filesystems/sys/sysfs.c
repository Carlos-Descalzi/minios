#include "lib/heap.h"
#include "fs/fs.h"
#include "lib/string.h"
#include "misc/debug.h"
/**
 * SYS File system
 **/
typedef struct {
    uint32_t inode;
    const char* name;
} FsDirentry;

typedef struct {
    Inode inode;
    uint8_t in_use;
} WorkInode;

#define WORK_INODES_COUNT   10
typedef struct {
    FileSystem fs;
    WorkInode work_inodes[WORK_INODES_COUNT];
    FsDirentry root[5];
} SysFileSystem;

#define SYSFS(fs)       ((SysFileSystem*)fs)

#define SYSFS_INODE_PROCESSES   3
#define SYSFS_INODE_DEVICES     4
#define SYSFS_INODE_FILESYSTEMS 5

static const char       FS_TYPE_NAME[] = "sys";
static FileSystemType   FS_TYPE;

static const char NAME_CWD[] = ".";
static const char NAME_PARENT[] = "..";
static const char NAME_PROCESSES[] = "processes";
static const char NAME_DEVICES[] = "devices";
static const char NAME_FILESYSTEMS[] = "filesystems";

static FsDirentry ROOT_DIR_ENTRIES[5];

static FileSystem*      create_fs                   (FileSystemType* fs_type, BlockDevice* device);
static void             list_inodes                 (FileSystem* fs, InodeVisitor visitor, void*data);
static void             close                       (FileSystem* fs);
static int32_t          load_inode                  (FileSystem* fs, uint32_t inodenum, Inode* inode);
static uint32_t         find_inode                  (FileSystem* fs, const char* path);
static int32_t          load                        (FileSystem* fs, Inode* inode, void* dest);
static uint32_t         read_block                  (FileSystem* fs, Inode* inode, 
                                                    uint32_t b_index, void* dest, 
                                                    uint32_t length);
static int32_t          get_direntry                (FileSystem* fs, Inode* inode, 
                                                    uint32_t* offset, DirEntry* direntry);
static int32_t          get_root_direntry           (FileSystem* fs, Inode* inode, 
                                                    uint32_t* offset, DirEntry* direntry);
static int32_t          get_processes_direntry      (FileSystem* fs, Inode* inode, 
                                                    uint32_t* offset, DirEntry* direntry);
static int32_t          get_devices_direntry        (FileSystem* fs, Inode* inode, 
                                                    uint32_t* offset, DirEntry* direntry);
static int32_t          get_filesystems_direntry    (FileSystem* fs, Inode* inode, 
                                                    uint32_t* offset, DirEntry* direntry);
static Inode*           alloc_inode                 (FileSystem* fs);
static void             free_inode                  (FileSystem* fs, Inode* inode);
static void             release_resources           (FileSystem* fs);

void module_init(){
    FS_TYPE.type_name = FS_TYPE_NAME;
    FS_TYPE.create = create_fs;
    fs_register_type(&FS_TYPE);
    ROOT_DIR_ENTRIES[0].inode = FS_INODE_ROOT_DIR;
    ROOT_DIR_ENTRIES[0].name = NAME_CWD;
    ROOT_DIR_ENTRIES[1].inode = FS_INODE_ROOT_DIR;
    ROOT_DIR_ENTRIES[1].name = NAME_PARENT;
    ROOT_DIR_ENTRIES[2].inode = SYSFS_INODE_PROCESSES;
    ROOT_DIR_ENTRIES[2].name = NAME_PROCESSES;
    ROOT_DIR_ENTRIES[3].inode = SYSFS_INODE_DEVICES;
    ROOT_DIR_ENTRIES[3].name = NAME_DEVICES;
    ROOT_DIR_ENTRIES[4].inode = SYSFS_INODE_FILESYSTEMS;
    ROOT_DIR_ENTRIES[4].name = NAME_FILESYSTEMS;
}

static FileSystem* create_fs(FileSystemType* fs_type, BlockDevice* device){
    if (!device){
        return NULL;
    }
    if (DEVICE(device)->kind != SYS){
        return NULL;
    }

    SysFileSystem* fs = heap_alloc(sizeof(SysFileSystem));

    FILE_SYSTEM(fs)->type = fs_type;
    FILE_SYSTEM(fs)->device = device;
    FILE_SYSTEM(fs)->list_inodes = list_inodes;
    FILE_SYSTEM(fs)->close = close;
    FILE_SYSTEM(fs)->find_inode = find_inode;
    FILE_SYSTEM(fs)->load_inode= load_inode;
    FILE_SYSTEM(fs)->load = load;
    FILE_SYSTEM(fs)->read_block = read_block;
    FILE_SYSTEM(fs)->get_direntry = get_direntry;
    FILE_SYSTEM(fs)->alloc_inode = alloc_inode;
    FILE_SYSTEM(fs)->free_inode = free_inode;
    //FILE_SYSTEM(fs)->stream_open = stream_open;
    FILE_SYSTEM(fs)->release_resources = release_resources;
    FILE_SYSTEM(fs)->inode_size = sizeof(Inode);
    FILE_SYSTEM(fs)->block_size = 1024;


    return FILE_SYSTEM(fs);
}

static void list_inodes(FileSystem* fs, InodeVisitor visitor, void*data){
}
static void close(FileSystem* fs){
    heap_free(fs);
}
static int32_t load_inode(FileSystem* fs, uint32_t inodenum, Inode* inode){
    inode->uid = inodenum;
    return 0;
}
static uint32_t find_inode(FileSystem* fs, const char* path){
    if (!strcmp(path,"/")){
        return FS_INODE_ROOT_DIR;
    } else if (!strcmp(path,"/devices")){
        return SYSFS_INODE_DEVICES;
    }
    return 0;
}
static int32_t load(FileSystem* fs, Inode* inode, void* dest){
    return 0;
}
static uint32_t read_block(FileSystem* fs, Inode* inode,
    uint32_t b_index, void* dest, uint32_t length){
    return 0;
}
static int32_t get_direntry(FileSystem* fs, Inode* inode, 
    uint32_t* offset, DirEntry* direntry){
    switch(inode->uid){
        case FS_INODE_ROOT_DIR:
            return get_root_direntry(fs, inode, offset, direntry);
        case SYSFS_INODE_PROCESSES:
            return get_processes_direntry(fs, inode, offset, direntry);
        case SYSFS_INODE_DEVICES:
            return get_devices_direntry(fs, inode, offset, direntry);
        case SYSFS_INODE_FILESYSTEMS:
            return get_filesystems_direntry(fs, inode, offset, direntry);
    }
    return -1;
}
static Inode* alloc_inode(FileSystem* fs){
    for (int i=0;i<WORK_INODES_COUNT;i++){
        if (!SYSFS(fs)->work_inodes[i].in_use){
            SYSFS(fs)->work_inodes[i].in_use = 1;
            return &(SYSFS(fs)->work_inodes[i].inode);
        }
    }
    return NULL;
}
static void free_inode(FileSystem* fs, Inode* inode){
    for (int i=0;i<WORK_INODES_COUNT;i++){
        if (&(SYSFS(fs)->work_inodes[i].inode) == inode){
            SYSFS(fs)->work_inodes[i].in_use = 0;
            break;
        }
    }
}
static void release_resources(FileSystem* fs){
    memset(&(SYSFS(fs)->work_inodes),0,sizeof(SYSFS(fs)->work_inodes));
}
static int32_t get_root_direntry(FileSystem* fs, Inode* inode, uint32_t* offset, DirEntry* direntry){
    if (*offset <= SYSFS_INODE_FILESYSTEMS){
        debug(ROOT_DIR_ENTRIES[*offset].name);debug("\n");
        strcpy(direntry->name,ROOT_DIR_ENTRIES[*offset].name);
        direntry->name_len = strlen(ROOT_DIR_ENTRIES[*offset].name);
        direntry->rec_len = direntry->name_len + sizeof(DirEntry);
        direntry->inode = ROOT_DIR_ENTRIES[*offset].inode;
        direntry->file_type = 2;
        (*offset)++;
    }
    return *offset > SYSFS_INODE_FILESYSTEMS;
}
static int32_t get_processes_direntry(FileSystem* fs, Inode* inode, uint32_t* offset, DirEntry* direntry){
    return 0;
}
static int32_t get_devices_direntry(FileSystem* fs, Inode* inode, uint32_t* offset, DirEntry* direntry){
    uint8_t kind;
    uint8_t instance;
    char buff[4];
    int dev_count = device_count_devices();

    if (*offset < dev_count +2){

        switch(*offset){
            case 0:
                strcpy(direntry->name, ".");
                direntry->file_type = 2;
                break;
            case 1:
                strcpy(direntry->name, "..");
                direntry->file_type = 2;
                break;
            default: 
                if (device_info(*offset - 2, &kind, &instance)){
                    return -1;
                }
                direntry->file_type = 1;
                strcpy(direntry->name, DEVICE_KIND_NAMES[kind]);
                strcat(direntry->name, itoa(instance,buff,10));
                break;
        }
        direntry->name_len = strlen(direntry->name);
        direntry->rec_len = direntry->name_len + sizeof(DirEntry);
        direntry->inode = inode->uid << 8 | *offset;

        (*offset)++;

    }   
    return *offset >= dev_count+2;

}
static int32_t get_filesystems_direntry(FileSystem* fs, Inode* inode, uint32_t* offset, DirEntry* direntry){
    return 0;
}
