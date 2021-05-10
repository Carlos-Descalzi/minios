#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "stdint.h"

#define DEVICE_TYPE_CHAR    1
#define DEVICE_TYPE_BLOCK   2

typedef struct Device{
    uint32_t 
        type:8,
        subtype: 24;
    int16_t     (*init)     (struct Device*);
    int16_t     (*setopt)   (struct Device*, uint32_t, void*);
    void        *device_params;
    size_t      device_params_length;
} Device;

typedef struct BlockDevice {
    Device base;
    int16_t     (*read)     (struct BlockDevice*,uint8_t*,uint16_t);
    int16_t     (*write)    (struct BlockDevice*,uint8_t*,uint16_t);
    int16_t     (*close)    (struct BlockDevice*);
    void        (*seek)     (struct BlockDevice*, uint32_t);
    void*       block_data;
} BlockDevice;

typedef struct CharDevice {
    Device base;
    int16_t     (*read)     (struct CharDevice*);
    int16_t     (*write)    (struct CharDevice*, uint8_t);
} CharDevice;

#define device_init(d)              (((Device*)d)->init((Device*)d))
#define device_setopt(d,o,v)        (((Device*)d)->setopt((Device*)d,o,v))

#define block_device_read(d,b,l)    (((BlockDevice*)d)->read((BlockDevice*)d,b,l))
#define block_device_write(d,b,l)   (((BlockDevice*)d)->write((BlockDevice*)d,b,l))
#define block_device_seek(d,p)      (((BlockDevice*)d)->seek((BlockDevice*)d,p))
#define block_device_close(d)       (((Blockdevice*)d)->close((BlockDevice*)d))

#define char_device_read(d)         (((CharDevice*)c)->read((CharDevice*)d))
#define char_device_write(d,c)      (((CharDevice*)c)->write((CharDevice*)d,c))

typedef uint16_t (*DeviceVisitor)(uint32_t, Device*);

void    device_list     (DeviceVisitor visitor);
void    device_register (Device* device, void* device_params);
#endif
