#include "io/streamimpl.h"
#include "lib/heap.h"
#include "lib/string.h"
#include "misc/debug.h"


static int16_t   read_byte     (Stream*);
static int16_t   write_byte    (Stream*,uint8_t);
static int16_t   read_bytes    (Stream*,uint8_t*,int16_t);
static int16_t   write_bytes   (Stream*,uint8_t*,int16_t);
static uint32_t  pos           (Stream*);
static int16_t   seek          (Stream*,uint32_t);
static uint32_t  size          (Stream*);
static void      close         (Stream*);
static void      flush         (Stream*);

Stream* char_array_stream_open (size_t buffer_size, int mode){

    CharArrayStream* stream = heap_alloc(sizeof(CharArrayStream) + buffer_size);

    stream->pos = 0;
    stream->buffer_size = buffer_size;
    memset(stream->buffer,0, buffer_size);

    STREAM(stream)->async = 0;
    STREAM(stream)->writeable = mode & O_WRONLY;
    STREAM(stream)->readable = mode & O_RDONLY;
    STREAM(stream)->read_byte = read_byte;
    STREAM(stream)->read_bytes = read_bytes;
    STREAM(stream)->write_byte = write_byte;
    STREAM(stream)->write_bytes = write_bytes;
    STREAM(stream)->pos = pos;
    STREAM(stream)->size = size;
    STREAM(stream)->seek = seek;
    STREAM(stream)->flush = flush;
    STREAM(stream)->close = close;

    return STREAM(stream);
}

static int16_t read_byte (Stream* stream){
    if (stream->readable){
        if (CHAR_ARRAY_STREAM(stream)->pos < CHAR_ARRAY_STREAM(stream)->buffer_size-1){
            return CHAR_ARRAY_STREAM(stream)->buffer[CHAR_ARRAY_STREAM(stream)->pos++];
        }
        
        return -1;
    }
    return 0;
}
static int16_t write_byte (Stream* stream,uint8_t data){
    if (stream->writeable){
        if (CHAR_ARRAY_STREAM(stream)->pos < CHAR_ARRAY_STREAM(stream)->buffer_size-1){
            CHAR_ARRAY_STREAM(stream)->buffer[CHAR_ARRAY_STREAM(stream)->pos++] = data;
            return 1;
        }
        return -1;
    }
    return 0;
}
static int16_t read_bytes (Stream* stream,uint8_t* buffer,int16_t len){
    if (stream->readable){
        int16_t available = CHAR_ARRAY_STREAM(stream)->buffer_size 
            - CHAR_ARRAY_STREAM(stream)->pos;

        if (available){
    
            int16_t to_read = min(len, available);

            memcpy(
                buffer, 
                CHAR_ARRAY_STREAM(stream)->buffer 
                + CHAR_ARRAY_STREAM(stream)->pos,
                to_read
            );

            CHAR_ARRAY_STREAM(stream)->pos += to_read;

            return to_read;
        }
    }
    return 0;
}
static int16_t write_bytes (Stream* stream,uint8_t* buffer,int16_t len){
    return 0;
}
static uint32_t pos (Stream* stream){
    return CHAR_ARRAY_STREAM(stream)->pos;
}
static int16_t seek (Stream* stream,uint32_t pos){
    return 0;
}
static uint32_t size (Stream* stream){
    return CHAR_ARRAY_STREAM(stream)->buffer_size;
}
static void close (Stream* stream){
    heap_free(stream);
}
static void flush (Stream* stream){
}
