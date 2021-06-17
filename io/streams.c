#include "misc/debug.h"
#include "io/streams.h"
#include "lib/heap.h"
#include "lib/string.h"

typedef struct {
    Stream stream;
    CharDevice* device;
    int mode;
} CharStream;

#define CHAR_STREAM(s)  ((CharStream*)s)

static int16_t read_byte(Stream* stream);
static int16_t write_byte(Stream* stream, uint8_t data);
static int16_t read_bytes(Stream* stream, uint8_t* buffer, int16_t size);
static int16_t write_bytes(Stream* stream, uint8_t* buffer, int16_t size);
static uint32_t pos (Stream* stream);
static int16_t seek (Stream* stream, uint32_t pos);
static uint32_t size (Stream* stream);
static void close(Stream* stream);

Stream* char_device_stream  (CharDevice* device, int mode){
    CharStream* stream = heap_alloc(sizeof(CharStream));
    memset(stream,0,sizeof(CharStream));
    STREAM(stream)->read_byte = read_byte;
    STREAM(stream)->read_bytes = read_bytes;
    STREAM(stream)->write_byte = write_byte;
    STREAM(stream)->write_bytes = write_bytes;
    STREAM(stream)->pos = pos;
    STREAM(stream)->seek = seek;
    STREAM(stream)->size = size;
    STREAM(stream)->close = close;

    stream->device = device;
    stream->mode = mode;

    return STREAM(stream);
}

static int16_t read_byte(Stream* stream){
    return char_device_read(CHAR_STREAM(stream)->device);
}

static int16_t write_byte(Stream* stream, uint8_t data){
    return char_device_write(CHAR_STREAM(stream)->device, data);
}

static int16_t read_bytes(Stream* stream, uint8_t* buffer, int16_t size){
    for (int i=0;i< size;i++){
        buffer[i] = char_device_read(CHAR_STREAM(stream)->device);
    }
    return size;
}

static int16_t write_bytes(Stream* stream, uint8_t* buffer, int16_t size){
    debug("writing bytes\n");
    debug_i(CHAR_STREAM(stream)->device,16);
    for (int i=0;i<size;i++){
        char_device_write(CHAR_STREAM(stream)->device,buffer[i]);
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
