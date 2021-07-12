#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "kernel/iobase.h"
#include "io/streams.h"
#include "lib/stdint.h"

#define DEVICE_TYPE_CHAR             1
#define DEVICE_TYPE_BLOCK            2

#define DEVICE_READ_ERROR           -1
#define DEVICE_READ_OK              0
#define DEVICE_READ_WAIT            1

/**
 * Device kinds supported
 **/
typedef enum {
    VIDEO   = 0,
    SER     = 1,
    DISK    = 2,
    NET     = 3,
    KBD     = 4,
    MOUSE   = 5,
    TERM    = 6,
    SYS     = 7
} DeviceKind;

extern const char* DEVICE_KIND_NAMES[];

typedef struct Device Device;

/**
 * Definition of a device type, has the initializer functions necessary
 * for instantiating device drivers
 **/
typedef struct DeviceType {
    uint8_t     kind;
    uint8_t     (*count_devices)    (struct DeviceType* device_type);
    Device*     (*instantiate)      (struct DeviceType* device_type, uint8_t device_number);
    void        (*release)          (struct DeviceType* device_type, Device* device);
} DeviceType;

/**
 * Very basic definition of a device
 **/
struct Device{
    uint32_t    type;
    uint8_t     kind;
    uint8_t     instance_number;
    uint8_t     async:1,
                mmapped:1,
                reserved:6;
    int16_t     (*setopt)           (struct Device*, uint32_t, void*);
    int16_t     (*getopt)           (struct Device*, uint32_t, void*);
    /**
     * If mmapped = 1, this function returns the base physical address for 
     * adressing this device
     **/
    uint32_t    (*base_address)     (struct Device*);
};

/**
 * This structure represents a block device
 **/
typedef struct BlockDevice {
    Device base;
    uint8_t     randomaccess:1,
                reserved:7;
    int16_t     (*read)             (struct BlockDevice*, uint8_t*,uint16_t);
    int16_t     (*read_async)       (struct BlockDevice*, IORequest* request);
    int16_t     (*write)            (struct BlockDevice*, uint8_t*,uint16_t);
    void        (*seek)             (struct BlockDevice*, uint32_t);
    void        (*flush)            (struct BlockDevice*);
    uint32_t    (*pos)              (struct BlockDevice*);
} BlockDevice;

/**
 * This structure defines a char device
 **/
typedef struct CharDevice {
    Device base;
    int16_t     (*read)             (struct CharDevice*);
    int16_t     (*read_async)       (struct CharDevice*, IORequest* request);
    int16_t     (*write)            (struct CharDevice*, uint8_t);
} CharDevice;

#define DEVICE(d)                       ((Device*)d)
#define CHAR_DEVICE(d)                  ((CharDevice*)d)
#define BLOCK_DEVICE(d)                 ((BlockDevice*)d)

#define device_setopt(d,o,v)            (DEVICE(d)->setopt(DEVICE(d),o,v))
#define device_getopt(d,o,v)            (DEVICE(d)->getopt(DEVICE(d),o,v))
#define device_base_address(d)          (DEVICE(d)->base_address(DEVICE(d)))

#define block_device_read(d,b,l)        (BLOCK_DEVICE(d)->read(BLOCK_DEVICE(d),b,l))
#define block_device_write(d,b,l)       (BLOCK_DEVICE(d)->write(BLOCK_DEVICE(d),b,l))
#define block_device_seek(d,p)          (BLOCK_DEVICE(d)->seek(BLOCK_DEVICE(d),p))
#define block_device_flush(d)           (BLOCK_DEVICE(d)->flush(BLOCK_DEVICE(d)))
#define block_device_pos(d)             (BLOCK_DEVICE(d)->pos(BLOCK_DEVICE(d)))
#define block_device_read_async(d,r)    (BLOCK_DEVICE(d)->read_async(BLOCK_DEVICE(d),r))

#define char_device_read(d)             (CHAR_DEVICE(d)->read(CHAR_DEVICE(d)))
#define char_device_read_async(d,r)     (CHAR_DEVICE(d)->read_async(CHAR_DEVICE(d),r))
#define char_device_write(d,c)          (CHAR_DEVICE(d)->write(CHAR_DEVICE(d),c))

typedef uint16_t (*DeviceTypeVisitor)   (uint32_t, DeviceType*,void*);
typedef uint16_t (*DeviceVisitor)       (uint32_t,uint8_t, Device* device,void*);

void    device_init                     (void);
void    device_list_types               (DeviceTypeVisitor visitor, void* data);
int16_t device_register_type            (DeviceType* device);
void    device_init_devices             (void);
int     device_count_devices            (void);
int     device_info                     (int index, uint8_t* kind, uint8_t* instance);
void    device_list                     (DeviceVisitor visitor, void *data);
Device* device_find                     (uint8_t kind, uint8_t instance);
Device* device_find_by_name             (const char* name);

#define device_find_by_id(id)           device_find((id)>>8,(id)&0xFF)
#define device_make_id(k,i)             ((k<<8)|i)
int     device_parse_name               (const char* name, uint8_t* kind, uint8_t* instance);

#endif
