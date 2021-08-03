#ifndef _STREAMIMPL_H_
#define _STREAMIMPL_H_
/**
 * Stream object for char devices
 **/

#include "io/streams.h"
#include "kernel/device.h"
#include "ipc/pipe.h"

typedef struct {
    Stream stream;
    Device* device;
} DeviceStream;

#define DEVICE_STREAM(s)                ((DeviceStream*)s)

typedef struct {
    Stream stream;
    uint32_t pos;
    size_t buffer_size;
    char buffer[];
} CharArrayStream;

#define CHAR_ARRAY_STREAM(s)            ((CharArrayStream*)s)

typedef struct {
    Stream stream;
    Pipe* pipe;
} PipeStream;

#define PIPE_STREAM(s)                  ((PipeStream*)s)

Stream* device_stream_open               (Device* device, int mode);
Stream* char_device_stream_open          (CharDevice* device, int mode);
Stream* block_device_stream_open         (BlockDevice* device, int mode);
Stream* char_array_stream_open           (size_t buffer_size, int mode);
Stream* pipe_stream_open                 (Pipe* pipe, int mode);

#endif
