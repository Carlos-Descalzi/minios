#ifndef _STREAMIMPL_H_
#define _STREAMIMPL_H_
/**
 * Stream object for char devices
 **/

#include "io/streams.h"
#include "kernel/device.h"

Stream* char_device_stream          (CharDevice* device, int mode);

#endif
