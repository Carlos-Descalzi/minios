#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "lib/stdint.h"

#define DEVICE_TYPE_CHAR             1
#define DEVICE_TYPE_BLOCK            2

typedef enum {
    VIDEO = 0,
    SER,
    DISK,
    NET,
    KBD,
    MOUSE
} DeviceKind;

typedef struct Device Device;

typedef struct DeviceType {
    uint8_t     kind;
    uint8_t     (*count_devices)    (struct DeviceType* device_type);
    Device*     (*instantiate)      (struct DeviceType* device_type, uint8_t device_number);
    void        (*release)          (struct DeviceType* device_type, Device* device);
} DeviceType;

struct Device{
    uint32_t    type;
    uint8_t     kind;
    uint8_t     instance_number;
    //int16_t     (*init)             (struct Device*);
    int16_t     (*setopt)           (struct Device*, uint32_t, void*);
};

typedef struct BlockDevice {
    Device base;
    int16_t     (*read)             (struct BlockDevice*,uint8_t*,uint16_t);
    int16_t     (*write)            (struct BlockDevice*,uint8_t*,uint16_t);
    int16_t     (*close)            (struct BlockDevice*);
    void        (*seek)             (struct BlockDevice*, uint32_t);
    void        (*flush)            (struct BlockDevice*);
    uint32_t    (*pos)              (struct BlockDevice*);
} BlockDevice;

typedef struct CharDevice {
    Device base;
    int16_t     (*read)             (struct CharDevice*);
    int16_t     (*write)            (struct CharDevice*, uint8_t);
} CharDevice;

#define DEVICE(d)                   ((Device*)d)
#define CHAR_DEVICE(d)              ((CharDevice*)d)
#define BLOCK_DEVICE(d)             ((BlockDevice*)d)
//#define device_init(d)              (((Device*)d)->init((Device*)d))
#define device_setopt(d,o,v)        (DEVICE(d)->setopt(DEVICE(d),o,v))

#define block_device_read(d,b,l)    (BLOCK_DEVICE(d)->read(BLOCK_DEVICE(d),b,l))
#define block_device_write(d,b,l)   (BLOCK_DEVICE(d)->write(BLOCK_DEVICE(d),b,l))
#define block_device_seek(d,p)      (BLOCK_DEVICE(d)->seek(BLOCK_DEVICE(d),p))
#define block_device_close(d)       (BLOCK_DEVICE(d)->close(BLOCK_DEVICE(d)))
#define block_device_flush(d)       (BLOCK_DEVICE(d)->flush(BLOCK_DEVICE(d)))
#define block_device_pos(d)         (BLOCK_DEVICE(d)->pos(BLOCK_DEVICE(d)))

#define char_device_read(d)         (CHAR_DEVICE(c)->read(CHAR_DEVICE(d)))
#define char_device_write(d,c)      (CHAR_DEVICE(c)->write(CHAR_DEVICE(d),c))

typedef uint16_t (*DeviceTypeVisitor)   (uint32_t, DeviceType*,void*);
typedef uint16_t (*DeviceVisitor)       (uint32_t,uint8_t, Device* device,void*);

void    device_init                     (void);
void    device_list_types               (DeviceTypeVisitor visitor, void* data);
int16_t device_register_type            (DeviceType* device);
void    device_init_devices             (void);
void    device_list                     (DeviceVisitor visitor, void *data);
Device* device_find                     (uint8_t kind, uint8_t instance);
#endif
