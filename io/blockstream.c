
#include "misc/debug.h"
#include "io/streamimpl.h"
#include "lib/heap.h"
#include "lib/string.h"

#define BUFFER_SIZE 1024

typedef struct {
    DeviceStream stream;
    int mode;
    uint16_t buffer_index;
    uint8_t buffer[BUFFER_SIZE];
} BlockStream;

#define BLOCK_STREAM(s) ((BlockStream*)s)

#define block_stream_device(s)      (BLOCK_DEVICE(DEVICE_STREAM(s)->device))

static int16_t read_bytes(Stream* stream, uint8_t* buffer, int16_t size);
static int16_t write_bytes(Stream* stream, uint8_t* buffer, int16_t size);
static int16_t read_async(Stream* stream, IORequest* request);
static uint32_t pos(Stream* stream);
static int16_t seek(Stream* stream, uint64_t pos, int whence);
static uint32_t size(Stream* stream);
static void close(Stream* stream);
static uint32_t available(Stream* stream);

Stream* block_device_stream_open(BlockDevice* device, int flags){

    BlockStream* stream = heap_new(BlockStream);
    memset(stream,0,sizeof(BlockStream));

    STREAM(stream)->type = STREAM_TYPE_BLOCKDEV;
    STREAM(stream)->async = DEVICE(device)->async;
    STREAM(stream)->seekable = device->randomaccess;
    STREAM(stream)->readable = (flags & O_RDONLY) != 0;
    STREAM(stream)->writeable = (flags & O_WRONLY) != 0;
    STREAM(stream)->nonblocking = (flags & O_NONBLOCK) != 0;
    STREAM(stream)->read_bytes = read_bytes;
    STREAM(stream)->read_async = read_async;
    STREAM(stream)->write_bytes = write_bytes;
    STREAM(stream)->pos = pos;
    STREAM(stream)->seek = seek;
    STREAM(stream)->size = size;
    STREAM(stream)->close = close;
    STREAM(stream)->available = available;

    DEVICE_STREAM(stream)->device = DEVICE(device);
    stream->buffer_index = 0;

    return STREAM(stream);
}

static int16_t read_bytes(Stream* stream, uint8_t* buffer, int16_t size){
    // FIXME
    if (stream_readable(stream)){
        return block_device_read(
            block_stream_device(stream),
            buffer,size);
    }
    return -1;
}

static int16_t write_bytes(Stream* stream, uint8_t* buffer, int16_t size){
    if (stream_writeable(stream)){
        return block_device_write(
            block_stream_device(stream),
            buffer,
            size
        );
    }
    return -1;
}

static uint32_t pos(Stream* stream){
    if (stream_seekable(stream)){
        return block_device_pos(block_stream_device(stream));
    }
    return -1;
}

static int16_t seek(Stream* stream, uint64_t pos, int whence){
    if (stream_seekable(stream)){
        block_device_seek(block_stream_device(stream), pos);
        return 0;
    }
    return -1;
}

static uint32_t size(Stream* stream){
    if (stream_seekable(stream)){
        // TODO
        return 0;
    }
    return -1;
}

static void close(Stream* stream){
    heap_free(stream);
}

static int16_t read_async(Stream* stream, IORequest* request){
    if (stream_async(stream)){
        return block_device_read_async(block_stream_device(stream), request);
    }
    return -1;
}
static uint32_t available(Stream* stream){
    return block_device_available(block_stream_device(stream));
}
