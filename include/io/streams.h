#ifndef _STREAMS_H_
#define _STREAMS_H_
/**
 * Definition of stream objects
 **/

#include "lib/stdlib.h"
#include "kernel/iobase.h"


/**
 * These correspond with fs file types
 **/
#define STREAM_TYPE_FIFO        0x1
#define STREAM_TYPE_CHARDEV     0x2
#define STREAM_TYPE_DIRECTORY   0x4
#define STREAM_TYPE_BLOCKDEV    0x6
#define STREAM_TYPE_FILE        0x8
#define STREAM_TYPE_SYMLINK     0xa
#define STREAM_TYPE_SOCKET      0xc

#define O_RDONLY        0x01
#define O_WRONLY        0x02
#define O_RDWR          0x03

#define O_CREAT         0x04
#define O_APPEND        0x08
#define O_TRUNC         0x10
#define O_NONBLOCK      0x20

#define EBADF           -11  // invalid descritor or not open for reading
#define EFAULT          -12  // buffer not in accessible space
#define EINTR           -13  // interrupted by a signal
#define EINVAL          -14  // not valid for reading
#define EISDIR          -15  // is directory
#define EIO             -16  // io error

typedef struct Stream {

    uint8_t     type;

    uint8_t     async:1,
                nonblocking:1,
                readable:1,
                writeable:1,
                seekable:1;

    int16_t     (*read_async)       (struct Stream*,IORequest* request);
    int16_t     (*read_bytes)       (struct Stream*,uint8_t*,int16_t);
    int16_t     (*write_bytes)      (struct Stream*,uint8_t*,int16_t);
    int16_t     (*write_async)      (struct Stream*,IORequest* request);
    void        (*flush)            (struct Stream*);
    uint32_t    (*pos)              (struct Stream*);
    int16_t     (*seek)             (struct Stream*,uint32_t);
    uint32_t    (*size)             (struct Stream*);
    void        (*close)            (struct Stream*);
    uint32_t    (*available)        (struct Stream*);
} Stream;

#define STREAM(s)                   ((Stream*)s)

#define stream_async(s)             (STREAM(s)->async)
#define stream_nonblocking(s)       (STREAM(s)->nonblocking)
#define stream_readable(s)          (STREAM(s)->readable)
#define stream_writeable(s)         (STREAM(s)->writeable)
#define stream_seekable(s)          (STREAM(s)->seekable)
#define stream_read_bytes(s,b,l)    (STREAM(s)->read_bytes(STREAM(s),b,l))
#define stream_write_bytes(s,b,l)   (STREAM(s)->write_bytes(STREAM(s),b,l))
#define stream_read_async(s,b)      (STREAM(s)->read_async(STREAM(s),b))
#define stream_write_async(s,b)     (STREAM(s)->write_async(STREAM(s),b))
#define stream_flush(s)             (STREAM(s)->flush(STREAM(s)))
#define stream_pos(s)               (STREAM(s)->pos(STREAM(s)))
#define stream_seek(s,p)            (STREAM(s)->seek(STREAM(s),p))
#define stream_close(s)             (STREAM(s)->close(STREAM(s)))
#define stream_available(s)         (STREAM(s)->available(STREAM(s)))

#define STREAM_READ     0x01
#define STREAM_WRITE    0x02

#endif
