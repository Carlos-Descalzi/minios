
#include "misc/debug.h"
#include "io/streamimpl.h"
#include "lib/heap.h"
#include "lib/string.h"

#define BUFFER_SIZE 1024

typedef struct {
    Stream stream;
    BlockDevice* device;
    int mode;
    uint16_t buffer_index;
    uint8_t buffer[BUFFER_SIZE];
} BlockStream;

#define BLOCK_STREAM(s) ((BlockStream*)s)

static int16_t read_byte(Stream* stream);
static int16_t write_byte(Stream* stream, uint8_t buffer);
static int16_t read_bytes(Stream* stream, uint8_t* buffer, int16_t size);
static int16_t write_bytes(Stream* stream, uint8_t* buffer, int16_t size);
static int16_t read_async(Stream* stream, IORequest* request);
static uint32_t pos(Stream* stream);
static int16_t seek(Stream* stream, uint32_t pos);
static uint32_t size(Stream* stream);
static void close(Stream* stream);

Stream* block_device_stream_open(BlockDevice* device, int mode){

    BlockStream* stream = heap_new(BlockStream);
    memset(stream,0,sizeof(BlockStream));

    STREAM(stream)->async = DEVICE(device)->async;
    STREAM(stream)->read_byte = read_byte;
    STREAM(stream)->read_bytes = read_bytes;
    STREAM(stream)->read_async = read_async;
    STREAM(stream)->write_byte = write_byte;
    STREAM(stream)->write_bytes = write_bytes;
    STREAM(stream)->pos = pos;
    STREAM(stream)->seek = seek;
    STREAM(stream)->size = size;
    STREAM(stream)->close = close;

    stream->device = device;
    stream->mode = mode;
    stream->buffer_index = 0;

    return STREAM(stream);
}

static int16_t read_byte(Stream* stream){
    // FIXME
    if (BLOCK_STREAM(stream)->buffer_index == 0
        || BLOCK_STREAM(stream)->buffer_index == BUFFER_SIZE){
        if (block_device_read(
            BLOCK_STREAM(stream)->device, 
            BLOCK_STREAM(stream)->buffer,
            BUFFER_SIZE
        ) < 0){
            return -1;
        }
    }
    return BLOCK_STREAM(stream)->buffer[BLOCK_STREAM(stream)->buffer_index++];
}

static int16_t write_byte(Stream* stream, uint8_t buffer){
    return block_device_write(
        BLOCK_STREAM(stream)->device,
        &buffer,
        1);
}

static int16_t read_bytes(Stream* stream, uint8_t* buffer, int16_t size){
    // FIXME
    return block_device_read(
        BLOCK_STREAM(stream)->device,
        buffer,size);
}

static int16_t write_bytes(Stream* stream, uint8_t* buffer, int16_t size){
    return block_device_write(
        BLOCK_STREAM(stream)->device,
        buffer,
        size
    );
}

static uint32_t pos(Stream* stream){
    return block_device_pos(BLOCK_STREAM(stream)->device);
}

static int16_t seek(Stream* stream, uint32_t pos){
    block_device_seek(BLOCK_STREAM(stream)->device, pos);
    return 0;
}

static uint32_t size(Stream* stream){
    return 0;
}

static void close(Stream* stream){
    heap_free(stream);
}

static int16_t read_async(Stream* stream, IORequest* request){
    return block_device_read_async(BLOCK_STREAM(stream)->device, request);
}
