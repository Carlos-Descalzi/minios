//#define NODEBUG
#include "kernel/device.h"
#include "board/io.h"
#include "board/pci.h"
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
#define SCREEN_OP_MODE          4

#define MODE_TEXT   0
#define MODE_ESCAPE 1

#define VBE_DISPI_IOPORT_INDEX      0x01ce
#define VBE_DISPI_IOPORT_DATA       0x01cf

#define VBE_DISPI_INDEX_XRES        0x01
#define VBE_DISPI_INDEX_YRES        0x02
#define VBE_DISPI_INDEX_BPP         0x03
#define VBE_DISPI_INDEX_ENABLE      0x04

#define VBE_DISPI_DISABLED          0x00
#define VBE_DISPI_ENABLED           0x01
#define VBE_DISPI_LFB_ENABLE        0x40
#define VBE_DISPI_NOCLEARMEM        0x80

#define MODE_TEXT       0
#define MODE_GRAPHIC    1

typedef struct {
    CharDevice device;
    uint32_t frame_buffer;
    uint8_t mode;
} ScreenDevice;

typedef struct Position {
    uint8_t x;
    uint8_t y;
} Position;

typedef struct {
    uint8_t mode;
    uint16_t width;
    uint16_t height;
    uint8_t bpp;
} ModeSetting;

#define         SCREEN_DEVICE(d)    ((ScreenDevice*)d)

static int16_t  screen_setopt       (Device* device, uint32_t option, void* data);
static int16_t  screen_getopt       (Device* device, uint32_t option, void* data);
static uint32_t screen_base_address (Device* device);
static int16_t  screen_read         (CharDevice* device);
static int16_t  screen_write        (CharDevice* device, uint8_t chr);
static uint8_t  count_devices       (DeviceType* device_type);
static Device*  instantiate         (DeviceType* device_type, uint8_t device_number);
static void     release             (DeviceType* device_type, Device* device);
static void     bga_write           (uint16_t index, uint16_t value);
static uint16_t bga_read            (uint16_t index);
static void     bga_set_mode        (uint16_t width, uint16_t height, uint8_t bpp, uint8_t linear, uint8_t clear);
static void     bga_set_text        (void);

static DeviceType DEVICE_TYPE;

static int visitor(uint8_t bus, uint8_t device, uint8_t func, PCIHeader* header, void* data){

    if (header->base.class == 0x03
        && header->base.subclass == 0x00
        && header->base.prog_if == 0x00){
        debug("Found PCI vga controller: ");

        if (header->base.vendor_id == 0x1234
            && header->base.device_id == 0x1111){
            debug("Bochs/QEMU display driver found");
        } else {
            debug("Unknown vendor id:");
            debug_i(header->base.vendor_id,16);
            debug(", device id:");
            debug_i(header->base.device_id,16);
            debug("\n");
        }

        if (header->base.header_type.type == 0x00){
            SCREEN_DEVICE(data)->frame_buffer = header->type00.base_addresses[0];
        }
    }

    return 0;
}

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

    pci_list_all_buses(visitor, device);
    
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

        case SCREEN_OP_MODE: {
            ModeSetting* mode = data;
            SCREEN_DEVICE(device)->mode = mode->mode;
            if (mode->mode == MODE_TEXT){
                bga_set_text();
            } else {
                bga_set_mode(mode->width, mode->height, mode->bpp, 1, 1);
            }
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
    if (SCREEN_DEVICE(device)->mode == MODE_TEXT){
        return 0xb8000;
    }
    return SCREEN_DEVICE(device)->frame_buffer;
}

static void bga_write(uint16_t index, uint16_t value){
    outw(VBE_DISPI_IOPORT_INDEX, index);
    outw(VBE_DISPI_IOPORT_DATA, value);
}

static uint16_t bga_read(uint16_t index){
    outw(VBE_DISPI_IOPORT_INDEX, index);
    return inw(VBE_DISPI_IOPORT_DATA);
}

static void bga_set_text(){
    bga_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
}


static void bga_set_mode(uint16_t width, uint16_t height, uint8_t bpp, uint8_t linear, uint8_t clear){
    bga_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
    bga_write(VBE_DISPI_INDEX_XRES, width);
    bga_write(VBE_DISPI_INDEX_YRES, height);
    bga_write(VBE_DISPI_INDEX_BPP, bpp);
    bga_write(VBE_DISPI_INDEX_ENABLE, 
            VBE_DISPI_ENABLED 
            | (linear ? VBE_DISPI_LFB_ENABLE : 0 )
            | (clear ? 0 : VBE_DISPI_NOCLEARMEM )
    );
}
