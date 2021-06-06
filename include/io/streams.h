#ifndef _STREAMS_H_
#define _STREAMS_H_

#include "lib/stdlib.h"

typedef struct Stream {
    int16_t     (*read_byte)        (struct Stream*);
    int16_t     (*write_byte)       (struct Stream*,uint8_t);
    int16_t     (*read_bytes)       (struct Stream*,uint8_t*,int16_t);
    int16_t     (*write_bytes)      (struct Stream*,uint8_t*,int16_t);
    uint32_t    (*pos)              (struct Stream*);
    int16_t     (*seek)             (struct Stream*,uint32_t);
    uint32_t    (*size)             (struct Stream*);
    void        (*close)            (struct Stream*);
} Stream;

#define STREAM(s)                   ((Stream*)s)

#define stream_read_byte(s)         (STREAM(s)->read_byte(STREAM(s)))
#define stream_write_byte(s,b)      (STREAM(s)->write_byte(STREAM(s),b))
#define stream_read_bytes(s,b,l)    (STREAM(s)->read_bytes(STREAM(s),b,l))
#define stream_write_bytes(s,b,l)   (STREAM(s)->write_bytes(STREAM(s),b,l))
#define stream_pos(s)               (STREAM(s)->pos(STREAM(s)))
#define stream_seek(s,p)            (STREAM(s)->seek(STREAM(s),p))
#define stream_close(s)             (STREAM(s)->close(STREAM(s)))



#endif