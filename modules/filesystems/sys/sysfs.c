//#define NODEBUG
#include "lib/heap.h"
#include "fs/fs.h"
#include "lib/string.h"
#include "misc/debug.h"
#include "io/streamimpl.h"
#include "board/memory.h"
#include "kernel/task.h"

#define SYSFS(fs)               ((SysFileSystem*)fs)
#define SYSFS_INODE(i)          ((SysFsInode*)i)
#define INODE(i)                ((Inode*)i)
#define WORK_INODES_COUNT       10

#define SYSFS_INODE_PROCESSES   3
#define SYSFS_INODE_DEVICES     4
#define SYSFS_INODE_FILESYSTEMS 5
#define SYSFS_INODE_MEMORY      6

#define SYSFS_INODE_MAX         5
/**
 * SYS File system
 **/
typedef struct {
    uint32_t inode;
    const char* name;
} FsDirentry;

typedef struct {
    Inode inode;
    uint32_t id;
    uint8_t in_use;
} SysFsInode;

typedef struct {
    FileSystem fs;
    SysFsInode work_inodes[WORK_INODES_COUNT];
    int process_count;
    uint32_t* process_list;
} SysFileSystem;


static FileSystemType   FS_TYPE;
static FsDirentry       ROOT_DIR_ENTRIES[6];

static const char       FS_TYPE_NAME[]              = "sys";

static const char       NAME_CWD[]                  = ".";
static const char       NAME_PARENT[]               = "..";
static const char       NAME_PROCESSES[]            = "processes";
static const char       NAME_DEVICES[]              = "devices";
static const char       NAME_FILESYSTEMS[]          = "filesystems";
static const char       NAME_MEMORY[]               = "memory";

static const char       DIRENT_KERNEL[]             = "kernel";
static const char       DIRENT_USER[]               = "user";

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
static int32_t          get_memory_direntry         (FileSystem* fs, Inode* inode, 
                                                    uint32_t* offset, DirEntry* direntry);
static Inode*           alloc_inode                 (FileSystem* fs);
static void             free_inode                  (FileSystem* fs, Inode* inode);
static void             release_resources           (FileSystem* fs);
static void             setup_direntry              (DirEntry* direntry, const char* name, 
                                                    int file_type, uint32_t inode);

static Stream*          open_stream                 (FileSystem* fs, uint32_t inodenum, uint32_t flags);
static Stream*          user_memory_stream          (FileSystem* fs, uint32_t flags);
static Stream*          kernel_memory_stream        (FileSystem* fs, uint32_t flags);
static Stream*          process_memory_stream       (FileSystem* fs, uint32_t flags, uint32_t task_id);
static void             get_process_list            (SysFileSystem* fs);

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
    ROOT_DIR_ENTRIES[5].inode = SYSFS_INODE_MEMORY;
    ROOT_DIR_ENTRIES[5].name = NAME_MEMORY;
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
    FILE_SYSTEM(fs)->release_resources = release_resources;
    FILE_SYSTEM(fs)->inode_size = sizeof(Inode);
    FILE_SYSTEM(fs)->block_size = 1024;
    FILE_SYSTEM(fs)->open_stream = open_stream;

    return FILE_SYSTEM(fs);
}

static void list_inodes(FileSystem* fs, InodeVisitor visitor, void*data){
}

static void close(FileSystem* fs){
    heap_free(fs);
}

static int32_t load_inode(FileSystem* fs, uint32_t inodenum, Inode* inode){
    SYSFS_INODE(inode)->id = inodenum;
    return 0;
}

static uint32_t find_inode(FileSystem* fs, const char* path){
    
    if (!strcmp(path,"/")){
        return FS_INODE_ROOT_DIR;
    } else if (!strcmp(path+1,NAME_DEVICES)){
        return SYSFS_INODE_DEVICES;
    } else if (!strncmp(path+1,NAME_MEMORY,strlen(NAME_MEMORY))){
        char* subpath = strchr(path+1,'/');
        if (subpath){
            if (!strcmp(subpath+1,DIRENT_KERNEL)){
                return SYSFS_INODE_MEMORY << 16 | 2;
            }
            if (!strcmp(subpath+1,DIRENT_USER)){
                return SYSFS_INODE_MEMORY << 16 | 3;
            }
        }
        return SYSFS_INODE_MEMORY;

    } else if (!strncmp(path+1,NAME_PROCESSES,strlen(NAME_PROCESSES))){
        char* subpath = strchr(path+1,'/');
        if (subpath){
            return SYSFS_INODE_PROCESSES << 16 | atoi(subpath+1);
        }
        return SYSFS_INODE_PROCESSES;
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

    switch(SYSFS_INODE(inode)->id){
        case FS_INODE_ROOT_DIR:
            return get_root_direntry(fs, inode, offset, direntry);
        case SYSFS_INODE_PROCESSES:
            return get_processes_direntry(fs, inode, offset, direntry);
        case SYSFS_INODE_DEVICES:
            return get_devices_direntry(fs, inode, offset, direntry);
        case SYSFS_INODE_FILESYSTEMS:
            return get_filesystems_direntry(fs, inode, offset, direntry);
        case SYSFS_INODE_MEMORY:
            return get_memory_direntry(fs, inode, offset, direntry);
    }
    return -1;
}

static Inode* alloc_inode(FileSystem* fs){
    for (int i=0;i<WORK_INODES_COUNT;i++){
        if (!SYSFS(fs)->work_inodes[i].in_use){
            SYSFS(fs)->work_inodes[i].in_use = 1;
            return INODE(&(SYSFS(fs)->work_inodes[i]));
        }
    }
    return NULL;
}

static void free_inode(FileSystem* fs, Inode* inode){
    for (int i=0;i<WORK_INODES_COUNT;i++){
        if (&(SYSFS(fs)->work_inodes[i]) == SYSFS_INODE(inode)){
            SYSFS(fs)->work_inodes[i].in_use = 0;
            break;
        }
    }
}

static void release_resources(FileSystem* fs){
    memset(&(SYSFS(fs)->work_inodes),0,sizeof(SYSFS(fs)->work_inodes));
}

static int32_t get_root_direntry(FileSystem* fs, Inode* inode, uint32_t* offset, DirEntry* direntry){
    debug("get root direntry\n");

    if (*offset <= SYSFS_INODE_MAX){
        debug(ROOT_DIR_ENTRIES[*offset].name);debug("\n");

        strcpy(direntry->name,ROOT_DIR_ENTRIES[*offset].name);
        direntry->name_len = strlen(ROOT_DIR_ENTRIES[*offset].name);
        direntry->rec_len = direntry->name_len + sizeof(DirEntry);
        direntry->inode = ROOT_DIR_ENTRIES[*offset].inode;
        direntry->file_type = 2;
        
        (*offset)++;
    } 

    return *offset > SYSFS_INODE_MAX;
}


static void setup_direntry(DirEntry* direntry, const char* name, int file_type, uint32_t inode){
    strcpy(direntry->name, name);
    direntry->file_type = file_type;
    direntry->name_len = strlen(direntry->name);
    direntry->rec_len = direntry->name_len + sizeof(DirEntry);
    direntry->inode = inode;
}

static int32_t get_processes_direntry(FileSystem* fs, Inode* inode, uint32_t* offset, DirEntry* direntry){

    int32_t finish = 0;

    switch(*offset){
        case 0:
            setup_direntry(direntry, NAME_CWD, 2, SYSFS_INODE(inode)->id);
            (*offset)++;
            break;
        case 1:
            setup_direntry(direntry, NAME_PARENT, 2, FS_INODE_ROOT_DIR);
            (*offset)++;
            break;
        default: {
                int proc_index = *offset - 2;
                char buff[10];

                if (SYSFS(fs)->process_count == 0){
                    get_process_list(SYSFS(fs));
                }

                int process_id = SYSFS(fs)->process_list[proc_index];

                itoa(process_id, buff, 10);
                setup_direntry(direntry, buff, 1, (SYSFS_INODE(inode)->id << 16) | process_id);

                (*offset)++;

                finish = proc_index >= SYSFS(fs)->process_count -1;

                if (finish){
                    SYSFS(fs)->process_count = 0;
                    heap_free(SYSFS(fs)->process_list);
                }
            }
            break;
    }

    return finish;
}

static int32_t get_devices_direntry(FileSystem* fs, Inode* inode, uint32_t* offset, DirEntry* direntry){
    uint8_t kind;
    uint8_t instance;
    char buff[4];
    char devname[10];
    int dev_count = device_count_devices();

    if (*offset < dev_count +2){

        switch(*offset){
            case 0:
                setup_direntry(direntry, NAME_CWD, 2, SYSFS_INODE(inode)->id);
                break;
            case 1:
                setup_direntry(direntry, NAME_PARENT, 2, FS_INODE_ROOT_DIR);
                break;
            default: 
                if (device_info(*offset - 2, &kind, &instance)){
                    return -1;
                }
                strcpy(devname, DEVICE_KIND_NAMES[kind]);
                strcat(devname, itoa(instance,buff,10));
                setup_direntry(direntry, devname, 1, SYSFS_INODE(inode)->id << 16 | *offset);
                break;
        }

        (*offset)++;

    }   
    return *offset >= dev_count+2;

}
static int32_t get_filesystems_direntry(FileSystem* fs, Inode* inode, uint32_t* offset, DirEntry* direntry){
    return 0;
}

static int32_t get_memory_direntry (FileSystem* fs, Inode* inode, uint32_t* offset, DirEntry* direntry){

    debug("get memory direntry\n");
    
    if (*offset < 4){
        switch(*offset){
            case 0:
                setup_direntry(direntry, NAME_CWD, 2, SYSFS_INODE(inode)->id);
                break;
            case 1:
                setup_direntry(direntry, NAME_PARENT, 2, FS_INODE_ROOT_DIR);
                break;
            case 2:
                setup_direntry(direntry, DIRENT_KERNEL, 1, SYSFS_INODE(inode)->id << 16 | *offset);
                break;
            case 3:
                setup_direntry(direntry, DIRENT_USER, 1, SYSFS_INODE(inode)->id << 16 | *offset);
                break;
        }

        (*offset)++;
    }

    return *offset >=4;

}

static Stream* open_stream (FileSystem* fs, uint32_t inodenum, uint32_t flags){
    if (inodenum >= 0x10000){
        uint32_t foldernum = inodenum >> 16;
        uint32_t filenum = inodenum & 0xFF;
        if (foldernum == SYSFS_INODE_MEMORY){
            switch(filenum){
                case 2:
                    return kernel_memory_stream(fs, flags);
                case 3:
                    return user_memory_stream(fs, flags);
            }
        } else if (foldernum == SYSFS_INODE_PROCESSES){
            return process_memory_stream(fs, flags, filenum);
        }
    }
    
    return NULL;
}

static Stream* user_memory_stream(FileSystem* fs, uint32_t flags){
    uint32_t total;
    uint32_t used;
    char buff[10];

    memory_stats(&total, &used);

    Stream* stream = char_array_stream_open(20, O_RDONLY);

    strcat(CHAR_ARRAY_STREAM(stream)->buffer, itoa(total * 4096,buff,10));
    strcat(CHAR_ARRAY_STREAM(stream)->buffer, ",");
    strcat(CHAR_ARRAY_STREAM(stream)->buffer, itoa(used* 4096,buff,10));
    strcat(CHAR_ARRAY_STREAM(stream)->buffer, "\n");

    return stream;
}

static Stream* kernel_memory_stream(FileSystem* fs, uint32_t flags){
    uint32_t total;
    uint32_t used;
    char buff[10];

    heap_stats(&total, &used);

    Stream* stream = char_array_stream_open(16, O_RDONLY);

    strcat(CHAR_ARRAY_STREAM(stream)->buffer, itoa(total,buff,10));
    strcat(CHAR_ARRAY_STREAM(stream)->buffer, ",");
    strcat(CHAR_ARRAY_STREAM(stream)->buffer, itoa(used,buff,10));
    strcat(CHAR_ARRAY_STREAM(stream)->buffer, "\n");

    return stream;
}
static Stream* process_memory_stream(FileSystem* fs, uint32_t flags, uint32_t task_id){

    Stream* stream = char_array_stream_open(256, O_RDONLY);
    memset(CHAR_ARRAY_STREAM(stream)->buffer,0,256);

    Task* task = tasks_get_task_by_tid(task_id);

    if (task->args){
        params_to_string(task->args, CHAR_ARRAY_STREAM(stream)->buffer);
    }
    strcat(CHAR_ARRAY_STREAM(stream)->buffer,"\n");

    return stream;
}

typedef struct {
    SysFileSystem* fs;
    int index;
} VisitorData;

static int get_tasks(Task* task, void* data){
    VisitorData* visitor_data = data;
    visitor_data->fs->process_list[visitor_data->index++] = task->tid;
    return 0;
}

static void get_process_list (SysFileSystem* fs){
    fs->process_count = tasks_count();

    fs->process_list = heap_alloc(fs->process_count * sizeof(uint32_t));

    VisitorData visitor_data = { .fs = fs, .index = 0 };

    tasks_iter_tasks(get_tasks, &visitor_data);
}

