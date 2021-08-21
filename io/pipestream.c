#include "io/streamimpl.h"
#include "misc/debug.h"
#include "lib/heap.h"

static int16_t   read_bytes    (Stream*,uint8_t*,int16_t);
static int16_t   read_async    (Stream*,IORequest*);
static int16_t   write_bytes   (Stream*,uint8_t*,int16_t);
static int16_t   write_async   (Stream*,IORequest*);
static uint32_t  pos           (Stream*);
static int16_t   seek          (Stream*,uint64_t,int);
static uint32_t  size          (Stream*);
static void      close         (Stream*);
static void      flush         (Stream*);


Stream* pipe_stream_open(Pipe* pipe, int mode){

    if ((mode & O_RDWR) == O_RDWR){
        debug("Pipe cannot be read and write at the same time\n");
        return NULL;
    }

    PipeStream* stream = heap_new(PipeStream);
    
    stream->pipe = pipe;
    pipe_ref(stream->pipe);
    STREAM(stream)->async = !(mode & O_NONBLOCK);
    STREAM(stream)->writeable = mode & O_WRONLY;
    STREAM(stream)->readable = mode & O_RDONLY;
    STREAM(stream)->read_bytes = read_bytes;
    STREAM(stream)->read_async = read_async;
    STREAM(stream)->write_bytes = write_bytes;
    STREAM(stream)->write_async = write_async;
    STREAM(stream)->pos = pos;
    STREAM(stream)->size = size;
    STREAM(stream)->seek = seek;
    STREAM(stream)->flush = flush;
    STREAM(stream)->close = close;

    return STREAM(stream);
}

static void close (Stream* stream){
    pipe_unref(PIPE_STREAM(stream)->pipe);
    heap_free(stream);
}
static uint32_t size (Stream* stream){
    return pipe_elem_count(PIPE_STREAM(stream)->pipe);
}
static uint32_t pos (Stream* stream){
    return 0;
}
static int16_t seek (Stream* stream,uint64_t pos,int whence){
    return 0;
}
static void flush (Stream* stream){
}
static int16_t read_bytes (Stream* stream,uint8_t* buffer,int16_t size){
    if (stream->readable){
    }
    return 0;
}
static int16_t write_bytes (Stream* stream,uint8_t* data,int16_t size){
    if (stream->writeable){
    }
    return 0;
}
static int16_t read_async (Stream* stream,IORequest* request){
    return 0;
}
static int16_t write_async (Stream* stream,IORequest* request){
    return 0;
}
