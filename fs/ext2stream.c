#define NODEBUG
#include "fs/ext2.h"
#include "lib/heap.h"
#include "lib/string.h"
#include "lib/stdlib.h"
#include "misc/debug.h"

typedef struct {
    Stream          stream;
    Ext2FileSystem* fs;
    Ext2Inode       inode; 
    uint8_t         mode;
    uint32_t        pos;
    uint32_t        numblocks;
    uint32_t        current_block;
    uint8_t*        block_buffer;
    char            path[1];
} FileStream;

#define     FILE_STREAM(fs)  ((FileStream*)fs)

int16_t     read_byte        (Stream*);
int16_t     write_byte       (Stream*,uint8_t);
int16_t     read_bytes       (Stream*,uint8_t*,int16_t);
int16_t     write_bytes      (Stream*,uint8_t*,int16_t);
uint32_t    pos              (Stream*);
int16_t     seek             (Stream*,uint32_t);
uint32_t    size             (Stream*);
void        close            (Stream*);

Stream* ext2_file_stream_open(Ext2FileSystem* fs, const char* path, uint8_t mode){
    uint32_t inodenum;
    FileStream* stream;

    debug("EXT2STREAM - Find inode for ");debug(path);debug("\n");
    inodenum = ext2_find_inode(fs, path);
    debug("EXT2STREAM - Inode: ");debug_i(inodenum,10);debug("\n");

    if (inodenum){
        stream = heap_alloc(sizeof(FileStream) + strlen(path));
        if (!stream){
            debug("EXT2STREAM - No memory for creating stream\n");
            return NULL;
        }
        memset(stream,0,sizeof(FileStream));
        stream->fs = fs;
        stream->block_buffer = heap_alloc(fs->block_size);
        ext2_load_inode(fs, inodenum, &(stream->inode));
        strcpy(stream->path, path);
        stream->mode = mode;
        stream->numblocks = stream->inode.size / fs->block_size;
        if (stream->inode.size % fs->block_size){
            stream->numblocks++;
        }
        STREAM(stream)->read_byte = read_byte;
        STREAM(stream)->write_byte = write_byte;
        STREAM(stream)->read_bytes = read_bytes;
        STREAM(stream)->write_bytes = write_bytes;
        STREAM(stream)->pos = pos;
        STREAM(stream)->seek = seek;
        STREAM(stream)->size = size;
        STREAM(stream)->close = close;
        stream->current_block = 0;
        debug("EXT2STREAM - reading blocks\n");
        ext2_read_block(fs, &(stream->inode), stream->current_block, stream->block_buffer, 0);
        return STREAM(stream);
    }
    return NULL;
}
void close(Stream* stream){
    heap_free(stream);
}

int16_t read_byte(Stream* stream){
    uint32_t rel_pos;
    uint32_t block_size;
    uint32_t block;
    uint8_t val;

    if (FILE_STREAM(stream)->pos >= FILE_STREAM(stream)->inode.size){
        return -1;
    }
    block_size = FILE_STREAM(stream)->fs->block_size;
    rel_pos = FILE_STREAM(stream)->pos % block_size;
    val = FILE_STREAM(stream)->block_buffer[rel_pos];

    FILE_STREAM(stream)->pos++;

    if (!(FILE_STREAM(stream)->pos % FILE_STREAM(stream)->fs->block_size)){
        FILE_STREAM(stream)->current_block++;
        block = FILE_STREAM(stream)->pos / FILE_STREAM(stream)->fs->block_size;
        ext2_read_block(
            FILE_STREAM(stream)->fs,  
            &(FILE_STREAM(stream)->inode),
            FILE_STREAM(stream)->current_block,
            FILE_STREAM(stream)->block_buffer,
            0
        );
    }

    return val;
}
int16_t write_byte(Stream* stream,uint8_t byte){
    return 0;
}
int16_t read_bytes(Stream* stream,uint8_t* bytes,int16_t size){
    uint32_t block_size;
    uint32_t offset;
    uint32_t block;
    uint16_t nblocks;
    uint32_t to_read;
    uint16_t i;
    uint32_t bytes_read;

    if (FILE_STREAM(stream)->pos >= FILE_STREAM(stream)->inode.size){
        return -1;
    }
    block_size = FILE_STREAM(stream)->fs->block_size;
    offset = FILE_STREAM(stream)->pos % block_size;
    block = FILE_STREAM(stream)->pos / block_size;
    nblocks = size / block_size + (size % block_size ? 1 : 0);
    bytes_read = 0;

    for (i=0;i<nblocks;i++){
        if (offset){
            to_read = min(size, block_size - offset);
            memcpy(bytes, FILE_STREAM(stream)->block_buffer + offset, to_read);
            offset = 0;
        } else {
            to_read = min(size, block_size);
            ext2_read_block(
                FILE_STREAM(stream)->fs,
                &(FILE_STREAM(stream)->inode),
                block + i,
                FILE_STREAM(stream)->block_buffer,
                FILE_STREAM(stream)->fs->block_size
            );
            memcpy(bytes + bytes_read, FILE_STREAM(stream)->block_buffer, to_read);
        }
        size -= to_read;
        bytes_read += to_read;
    }
    FILE_STREAM(stream)->pos += bytes_read;

    return bytes_read;
}
int16_t write_bytes(Stream* stream,uint8_t* bytes,int16_t size){
    return 0;
}
uint32_t pos(Stream* stream){
    return FILE_STREAM(stream)->pos;
}
int16_t seek(Stream* stream,uint32_t pos){
    if (pos > FILE_STREAM(stream)->inode.size -1){
        return -1;
    }
    FILE_STREAM(stream)->pos = pos;
    return 0;
}
uint32_t size(Stream* stream){
    return FILE_STREAM(stream)->inode.size;
}

