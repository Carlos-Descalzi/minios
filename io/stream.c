
#include "io/streamimpl.h"

Stream* device_stream_open (Device* device, int mode){
    if (device->type == DEVICE_TYPE_CHAR){
        return char_device_stream_open(CHAR_DEVICE(device), mode);
    } else {
        return block_device_stream_open(BLOCK_DEVICE(device), mode);
    }
}
