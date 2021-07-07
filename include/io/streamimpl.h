#ifndef _STREAMIMPL_H_
#define _STREAMIMPL_H_
/**
 * Stream object for char devices
 **/

#include "io/streams.h"
#include "kernel/device.h"

Stream* device_stream_open               (Device* device, int mode);
Stream* char_device_stream_open          (CharDevice* device, int mode);
Stream* block_device_stream_open         (BlockDevice* device, int mode);

#endif
