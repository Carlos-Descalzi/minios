#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "stdint.h"

#define DEVICE_TYPE_CHAR             1
#define DEVICE_TYPE_BLOCK            2

#define DEVICE_BUS_TYPE_MAINBOARD    1

#define DEVICE_BUS_SUBTYPE_IOPORT    1
#define DEVICE_BUS_SUBTYPE_MMAPPED   2
#define DEVICE_BUS_SUBTYPE_PS2       3

#define DEVICE_BUS_TYPE_PCI          2
#define DEVICE_BUS_TYPE_USB          3

typedef enum {
    CON = 0,
    SER,
    HD,
    NET
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
    void*       block_data;
} BlockDevice;

typedef struct CharDevice {
    Device base;
    int16_t     (*read)             (struct CharDevice*);
    int16_t     (*write)            (struct CharDevice*, uint8_t);
} CharDevice;

#define DEVICE(d)                   ((Device*)d)
//#define device_init(d)              (((Device*)d)->init((Device*)d))
#define device_setopt(d,o,v)        (((Device*)d)->setopt((Device*)d,o,v))

#define block_device_read(d,b,l)    (((BlockDevice*)d)->read((BlockDevice*)d,b,l))
#define block_device_write(d,b,l)   (((BlockDevice*)d)->write((BlockDevice*)d,b,l))
#define block_device_seek(d,p)      (((BlockDevice*)d)->seek((BlockDevice*)d,p))
#define block_device_close(d)       (((Blockdevice*)d)->close((BlockDevice*)d))

#define char_device_read(d)         (((CharDevice*)c)->read((CharDevice*)d))
#define char_device_write(d,c)      (((CharDevice*)c)->write((CharDevice*)d,c))

typedef uint16_t (*DeviceVisitor)(uint32_t, DeviceType*,void*);

void    device_init             (void);
void    device_list_types       (DeviceVisitor visitor, void*user_data);
int16_t device_register_type    (DeviceType* device);
void    device_init_devices     (void);
#endif
