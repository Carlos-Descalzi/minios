#include <stdio.h>
#include "ext2.h"

static void open_image(BlockDevice* device, const char* filename);


int main(int argc, char**argv){
    BlockDevice device;
    Ext2FileSystem fs;

    open_image(&device,"samplee2fs.img");

    ext2_open(&device,&fs);
    printf("Block size: %u\n", fs.block_size);
    printf("Block count: %u\n", fs.super_block.block_count);
    printf("Inode count: %u\n", fs.super_block.inode_count);


    return 0;
}
static int16_t image_read(BlockDevice* device, uint8_t* buffer, uint16_t length){
    size_t l;

    return fread(buffer, length, 1, (FILE*)device->block_data);
}

static int16_t image_write(BlockDevice* device, uint8_t* buffer, uint16_t length){
    return fwrite(buffer, length, 1, (FILE*)device->block_data);
}
static int16_t image_close(BlockDevice* device){
    fclose((FILE*)device->block_data);
}
static void image_seek(BlockDevice* device, uint32_t pos){
    fseek((FILE*)device->block_data, pos, SEEK_SET);
}
static void open_image(BlockDevice* device, const char* filename){
    device->block_data = fopen(filename,"rb");
    device->read = image_read;
    device->write = image_write;
    device->close = image_close;
    device->seek = image_seek;
}


