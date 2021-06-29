#ifndef _FS_H_
#define _FS_H_

#include "lib/stdint.h"
#include "io/streams.h"
#include "kernel/device.h"

typedef struct Inode {
    uint16_t mode:12,
             type:4;
    uint16_t uid;
    uint32_t size;
    uint32_t atime;
    uint32_t ctime;
    uint32_t mtime;
    uint32_t dtime;
    uint16_t gid;
    uint16_t link_count;
    uint32_t block_count;
    uint32_t flags;
} Inode;

typedef struct DirEntry {
    uint32_t inode;
    uint16_t rec_len;
    uint8_t name_len;
    uint8_t file_type;
    char name[1];
} DirEntry;

typedef struct FileSystemType FileSystemType;
typedef struct FileSystem FileSystem;

typedef int8_t (*InodeVisitor)(FileSystem*,uint32_t,Inode* inode, void*);

struct FileSystemType {
    char* type_name;
    FileSystem* (*create)       (struct FileSystemType*, BlockDevice*);
};

struct FileSystem {
    FileSystemType* type;
    BlockDevice* device;

    uint32_t    inode_size;
    uint32_t    block_size;

    void        (*list_inodes)          (struct FileSystem*, InodeVisitor, void*);
    void        (*close)                (struct FileSystem*);
    uint32_t    (*find_inode)           (struct FileSystem*, const char*);
    int32_t     (*load_inode)           (struct FileSystem*, uint32_t, Inode*);
    int32_t     (*load)                 (struct FileSystem*, Inode*, void*);
    uint32_t    (*read_block)           (struct FileSystem*, Inode*, uint32_t, void*, uint32_t);
    int32_t     (*get_direntry)         (struct FileSystem*, Inode*, uint32_t*, DirEntry*);
    Inode*      (*alloc_inode)          (struct FileSystem*);
    void        (*free_inode)           (struct FileSystem*, Inode*);
    Stream*     (*stream_open)          (struct FileSystem*, const char*, uint8_t);
    void        (*release_resources)    (struct FileSystem*);

};

void            fs_init                     (void);
void            fs_register_type            (FileSystemType* fs_type);
FileSystem*     fs_get_filesystem           (BlockDevice* device);
void            fs_release_filesystem       (FileSystem* fs);

#define         FILE_SYSTEM(f)              ((FileSystem*)(f))

#define         fs_type_create(fst,d)       ((fst)->create(fst, d))
#define         fs_list_inodes(fs,v)        ((fs)->list_inodes(fs, v))
#define         fs_close(fs)                ((fs)->close(fs))
#define         fs_find_inode(fs,p)         ((fs)->find_inode(fs,p))
#define         fs_load_inode(fs,n,i)       ((fs)->load_inode(fs,n,i))
#define         fs_load(fs,i,b)             ((fs)->load(fs,i,b))
#define         fs_read_block(fs,i,n,b,s)   ((fs)->read_block(fs,i,n,b,s))
#define         fs_get_direntry(fs,i,o,d)   ((fs)->get_direntry(fs,i,o,d))
#define         fs_alloc_inode(fs)          ((fs)->alloc_inode(fs))
#define         fs_free_inode(fs,i)         ((fs)->free_inode(fs,i))
#define         fs_file_stream_open(fs,p,m) ((fs)->stream_open(fs,p,m))
#define         fs_release_resources(fs)    ((fs)->release_resources(fs))

#endif
