//#define NODEBUG
#include "kernel/device.h"
#include "board/io.h"
//#include "board/console.h"
#include "lib/string.h"
#include "lib/stdlib.h"
#include "misc/debug.h"
#include "lib/heap.h"

/**
 * This device is just a wrapper over raw console API.
 * adding basic ANSI terminal support.
 * TODO: Move all this logic to console driver
 **/

#define DEVICE_SUBTYPE_SCREEN   1

#define SCREEN_OPT_CURSOR_ON    1
#define SCREEN_OPT_CURSOR_OFF   2
#define SCREEN_OPT_CURSOR_POS   3

#define MODE_TEXT   0
#define MODE_ESCAPE 1

typedef struct {
    CharDevice device;
} ScreenDevice;

typedef struct Position {
    uint8_t x;
    uint8_t y;
} Position;

#define         SCREEN_DEVICE(d)    ((ScreenDevice*)d)

static int16_t  screen_setopt       (Device* device, uint32_t option, void* data);
static int16_t  screen_getopt       (Device* device, uint32_t option, void* data);
static uint32_t screen_base_address (Device* device);
static int16_t  screen_read         (CharDevice* device);
static int16_t  screen_write        (CharDevice* device, uint8_t chr);
static uint8_t  count_devices       (DeviceType* device_type);
static Device*  instantiate         (DeviceType* device_type, uint8_t device_number);
static void     release             (DeviceType* device_type, Device* device);

static DeviceType DEVICE_TYPE;

void module_init(){
    DEVICE_TYPE.kind = VIDEO;
    DEVICE_TYPE.count_devices = count_devices;
    DEVICE_TYPE.instantiate = instantiate;
    DEVICE_TYPE.release = release;
    device_register_type((DeviceType*)&DEVICE_TYPE);
    debug("** Screen device type registered\n");
}

static uint8_t count_devices(DeviceType* device_type){
    return 1;
}

static Device* instantiate(DeviceType* device_type, uint8_t device_number){

    ScreenDevice* device = heap_new(ScreenDevice);
    
    DEVICE(device)->type = DEVICE_TYPE_CHAR;
    DEVICE(device)->kind = VIDEO;
    DEVICE(device)->async = 0;
    DEVICE(device)->mmapped = 1;
    DEVICE(device)->setopt = screen_setopt;
    DEVICE(device)->getopt = screen_getopt;
    DEVICE(device)->base_address = screen_base_address;
    CHAR_DEVICE(device)->read = screen_read;
    CHAR_DEVICE(device)->write = screen_write;

    return DEVICE(device);
}

static void release(DeviceType* device_type, Device* device){
    heap_free(device);
}

static void cursor_on(){
    outb(0x3d4, 0x0a);
    outb(0x3d5, (inb(0x3d5) & 0xC0) | 0x1);
    outb(0x3d4, 0x0b);
    outb(0x3d5, (inb(0x3d5) & 0xE0) | 0xF);
}

static void cursor_off(){
    outb(0x3d4, 0x0a);
    outb(0x3d5, 0x20);
}

static void set_cursor_pos(uint16_t pos){
    outb(0x3d4,0x0F);
    outb(0x3d5,pos & 0xFF);
    outb(0x3d4,0x0E);
    outb(0x3d5,(pos >> 8) & 0xFF);
}

static int16_t screen_setopt(Device* device, uint32_t option, void* data){
    switch(option){
        case SCREEN_OPT_CURSOR_ON:
            cursor_on();
            return 0;
        case SCREEN_OPT_CURSOR_OFF:
            cursor_off();
            return 0;

        case SCREEN_OPT_CURSOR_POS: {
            Position* position = data;
            set_cursor_pos(position->y * 80 + position->x);
            cursor_on();
            return 0;
        }
    }
    return -1;
}
static int16_t screen_getopt (Device* device, uint32_t option, void* data){
    switch (option){
        case SCREEN_OPT_CURSOR_POS: {
            return 0;
        }
    }
    return -1;
}

static int16_t screen_read(CharDevice* device){
    return 0;
}

static int16_t screen_write(CharDevice* device, uint8_t chr){
    return 0;
}

static uint32_t screen_base_address (Device* device){
    return 0xb8000;
}
