#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "stdint.h"

typedef struct BlockDevice {
    int16_t (*read)(struct BlockDevice*,uint8_t*,uint16_t);
    int16_t (*write)(struct BlockDevice*,uint8_t*,uint16_t);
    int16_t (*close)(struct BlockDevice*);
    void    (*seek)(struct BlockDevice*, uint32_t);
    void* block_data;
} BlockDevice;

#define block_device_read(d,b,l)    (d->read(d,b,l))
#define block_device_write(d,b,l)   (d->write(d,b,l))
#define block_device_seek(d,p)      (d->seek(d,p))
#define block_device_close(d)       (d->close(d))


#endif
