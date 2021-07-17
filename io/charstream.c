#define NODEBUG
#include "misc/debug.h"
#include "io/streamimpl.h"
#include "lib/heap.h"
#include "lib/string.h"

typedef struct {
    DeviceStream stream;
    int mode;
} CharStream;

#define CHAR_STREAM(s)              ((CharStream*)s)

#define char_stream_device(s)       (CHAR_DEVICE(DEVICE_STREAM(s)->device))

static int16_t read_byte(Stream* stream);
static int16_t read_async(Stream* stream, IORequest* request);
static int16_t write_byte(Stream* stream, uint8_t data);
static int16_t read_bytes(Stream* stream, uint8_t* buffer, int16_t size);
static int16_t write_bytes(Stream* stream, uint8_t* buffer, int16_t size);
static uint32_t pos (Stream* stream);
static int16_t seek (Stream* stream, uint32_t pos);
static uint32_t size (Stream* stream);
static void close(Stream* stream);

Stream* char_device_stream_open  (CharDevice* device, int flags){

    CharStream* stream = heap_new(CharStream);
    memset(stream,0,sizeof(CharStream));

    STREAM(stream)->type = STREAM_TYPE_CHARDEV;
    STREAM(stream)->async = DEVICE(device)->async;
    STREAM(stream)->readable = (flags & O_RDONLY) != 0;
    STREAM(stream)->writeable = (flags & O_WRONLY) != 0;
    STREAM(stream)->nonblocking = (flags & O_NONBLOCK) != 0;
    STREAM(stream)->seekable = 0;
    STREAM(stream)->read_byte = read_byte;
    STREAM(stream)->read_async = read_async;
    STREAM(stream)->read_bytes = read_bytes;
    STREAM(stream)->write_byte = write_byte;
    STREAM(stream)->write_bytes = write_bytes;
    STREAM(stream)->pos = pos;
    STREAM(stream)->seek = seek;
    STREAM(stream)->size = size;
    STREAM(stream)->close = close;

    DEVICE_STREAM(stream)->device = DEVICE(device);

    return STREAM(stream);
}

static int16_t read_byte(Stream* stream){
    return char_device_read(char_stream_device(stream));
}

static int16_t write_byte(Stream* stream, uint8_t data){
    return char_device_write(char_stream_device(stream), data);
}
static int16_t read_async(Stream* stream, IORequest* request){
    debug("Char stream read async ");
    debug_i(CHAR_DEVICE(char_stream_device(stream))->read_async,16);
    debug("\n");
    return char_device_read_async(char_stream_device(stream), request);
}

static int16_t read_bytes(Stream* stream, uint8_t* buffer, int16_t size){
    for (int i=0;i< size;i++){
        buffer[i] = char_device_read(char_stream_device(stream));
    }
    return size;
}

static int16_t write_bytes(Stream* stream, uint8_t* buffer, int16_t size){
    for (int i=0;i<size;i++){
        char_device_write(char_stream_device(stream),buffer[i]);
    }
    return size;
}
static uint32_t pos (Stream* stream){
    return 0;
}
static int16_t seek (Stream* stream, uint32_t pos){
    return 0;
}
static uint32_t size (Stream* stream){
    return 0;
}
static void close(Stream* stream){
    heap_free(stream);
}
