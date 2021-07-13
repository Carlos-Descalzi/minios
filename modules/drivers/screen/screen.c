//#define NODEBUG
#include "kernel/device.h"
#include "board/io.h"
#include "board/pci.h"
//#include "board/console.h"
#include "lib/string.h"
#include "lib/stdlib.h"
#include "misc/debug.h"
#include "lib/heap.h"
#include "font.h"
#include "kernel/paging.h"
#include "board/memory.h"

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
#define SCREEN_OPT_POS          5
#define SCREEN_OPT_CLEAR        6
#define SCREEN_OPT_SCROLL       7

#define MODE_TEXT   0
#define MODE_ESCAPE 1

#define VBE_DISPI_IOPORT_INDEX      0x01ce
#define VBE_DISPI_IOPORT_DATA       0x01cf

#define VBE_DISPI_INDEX_XRES        0x01
#define VBE_DISPI_INDEX_YRES        0x02
#define VBE_DISPI_INDEX_BPP         0x03
#define VBE_DISPI_INDEX_ENABLE      0x04

#define VBE_DISPI_INDEX_VIRT_WIDTH  0x06
#define VBE_DISPI_INDEX_VIRT_HEIGHT 0x07
#define VBE_DISPI_INDEX_X_OFFSET    0x08
#define VBE_DISPI_INDEX_Y_OFFSET    0x09

#define VBE_DISPI_DISABLED          0x00
#define VBE_DISPI_ENABLED           0x01
#define VBE_DISPI_LFB_ENABLE        0x40
#define VBE_DISPI_NOCLEARMEM        0x80

#define MODE_TEXT       0
#define MODE_GRAPHIC    1

#define TEXT_MODE_XRES          640
#define TEXT_MODE_YRES          400
#define TEXT_MODE_BPP           4
#define TEXT_MODE_ROW_WIDTHDW   (TEXT_MODE_XRES / 8)
#define TEXT_MODE_ROW_WIDTH     (TEXT_MODE_ROW_WIDTHDW * sizeof(uint32_t))
#define TEXT_MODE_MEM_SIZE      (TEXT_MODE_ROW_WIDTH * TEXT_MODE_YRES)
#define TEXT_MODE_CHAR_HEIGHT   16
#define LINESIZE                ( 80 * 16 )

typedef struct {
    CharDevice device;
    void* frame_buffer;
    uint32_t frame_buffer_phys_address;
    uint8_t mode;
    uint32_t pos;
    uint8_t input_buffer[2];
    uint8_t buff_index;
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
static void     bga_set_mode        (uint16_t width, uint16_t height, uint8_t bpp);
static void     bga_set_text        (void);
static void     write_char          (ScreenDevice* device, uint8_t chr);
static void     flip_cursor         (ScreenDevice* device);
static void     clear_screen        (ScreenDevice* device);
static void     do_scroll           (ScreenDevice* device);
static void     set_mode            (ScreenDevice* device, ModeSetting* mode_setting);

static DeviceType DEVICE_TYPE;

static int visitor(uint8_t bus, uint8_t device, uint8_t func, PCIHeader* header, void* data){

    if (header->base.class == 0x03
        && header->base.subclass == 0x00
        && header->base.prog_if == 0x00
        && header->base.vendor_id == 0x1234
        && header->base.device_id == 0x1111){
        debug("Found PCI vga controller: Bochs/QEMU display driver found\n");

        if (header->base.header_type.type == 0x00){
            SCREEN_DEVICE(data)->frame_buffer_phys_address = header->type00.base_addresses[0];
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

    device->buff_index = 0;
    device->pos = 0;

    device->frame_buffer = (void*) paging_map_to_task(
        (PageDirectoryEntry*)KERNEL_PAGE_DIR_ADDRESS, 
        device->frame_buffer_phys_address,
        TEXT_MODE_MEM_SIZE,
        0
    );

    bga_set_text();
    clear_screen(device);

    return DEVICE(device);
}

static void release(DeviceType* device_type, Device* device){
    heap_free(device);
}

static int16_t screen_setopt(Device* device, uint32_t option, void* data){
    switch(option){

        case SCREEN_OPT_POS:
            flip_cursor(SCREEN_DEVICE(device));
            SCREEN_DEVICE(device)->pos = (uint32_t)data;
            flip_cursor(SCREEN_DEVICE(device));
            return 0;

        case SCREEN_OPT_CLEAR:
            clear_screen(SCREEN_DEVICE(device));
            return 0;

        case SCREEN_OPT_SCROLL:
            do_scroll(SCREEN_DEVICE(device));
            return 0;

        case SCREEN_OP_MODE: 
            set_mode(SCREEN_DEVICE(device), (ModeSetting*) data);
            return 0;
    }
    return -1;
}

static int16_t screen_getopt (Device* device, uint32_t option, void* data){
    switch (option){
        case SCREEN_OPT_CURSOR_POS: 
            return 0;
        case SCREEN_OPT_POS:
            *((uint32_t*)data) = SCREEN_DEVICE(device)->pos;
            return 0;
    }
    return -1;
}

static int16_t screen_read(CharDevice* device){ 
    return 0; 
}

static int16_t screen_write(CharDevice* device, uint8_t chr){ 
    if (SCREEN_DEVICE(device)->mode == MODE_TEXT){
        write_char(SCREEN_DEVICE(device), chr);
    } 
    return 0; 
}

static uint32_t screen_base_address (Device* device){
    if (SCREEN_DEVICE(device)->mode == MODE_TEXT){
        return 0;
    } 
    return SCREEN_DEVICE(device)->frame_buffer_phys_address;
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
    bga_set_mode(
        TEXT_MODE_XRES,
        TEXT_MODE_YRES,
        TEXT_MODE_BPP
    );
}

static void bga_set_mode(uint16_t width, uint16_t height, uint8_t bpp){
    bga_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
    bga_write(VBE_DISPI_INDEX_XRES, width);
    bga_write(VBE_DISPI_INDEX_VIRT_WIDTH, width);
    bga_write(VBE_DISPI_INDEX_YRES, height);
    bga_write(VBE_DISPI_INDEX_VIRT_HEIGHT, height);
    bga_write(VBE_DISPI_INDEX_X_OFFSET, 0);
    bga_write(VBE_DISPI_INDEX_Y_OFFSET, 0);
    bga_write(VBE_DISPI_INDEX_BPP, bpp);
    bga_write(VBE_DISPI_INDEX_ENABLE, 
            VBE_DISPI_ENABLED 
            | VBE_DISPI_LFB_ENABLE 
    );
}

static uint32_t calc_value(uint8_t color, uint8_t row){
    uint32_t val = 0;

    color &= 0x7;

    if (color & 1){
        val |= row;
    } 
    if (color & 2){
        val |= row << 8;
    }
    if (color & 4){
        val |= row << 16;
    }
    if (color & 8){
        val |= row << 24;
    }

    return val;
}

static void write_char(ScreenDevice* device, uint8_t val){

    device->input_buffer[device->buff_index++] = val;

    if (device->buff_index == 2){
        flip_cursor(device);

        uint8_t color = device->input_buffer[0];
        uint8_t chr = device->input_buffer[1];

        uint32_t* frame_buffer = device->frame_buffer;

        const uint8_t* chr_font = VGA_FONT_16 + chr * TEXT_MODE_CHAR_HEIGHT;

        uint16_t x_pos = (device->pos % 80) ;
        uint16_t y_pos = (device->pos / 80) ;
        uint32_t pos = y_pos * (80 * 16) + x_pos;

        for (int i=0;i<TEXT_MODE_CHAR_HEIGHT;i++){
            uint32_t p = pos + i * TEXT_MODE_ROW_WIDTHDW;
            uint8_t row = chr_font[i];
            frame_buffer[p] = calc_value(color, row);
        }

        device->buff_index = 0;
        device->pos++;

        if (device->pos >= 80 * 25){
            device->pos-=80;
        }

        flip_cursor(device);
    }
}

static void flip_cursor(ScreenDevice* device){

    uint32_t* frame_buffer = device->frame_buffer;

    uint16_t x_pos = (device->pos % 80 ) ;
    uint16_t y_pos = (device->pos / 80 ) ;
    uint16_t pos = y_pos * LINESIZE + x_pos;

    for (int i=0;i<TEXT_MODE_CHAR_HEIGHT;i++){
        uint32_t p = pos + i * TEXT_MODE_ROW_WIDTHDW;
        frame_buffer[p] = ~frame_buffer[p];
    }

}

static void clear_screen(ScreenDevice* device){

    device->pos = 0;

    void* frame_buffer = device->frame_buffer;

    debug_i(frame_buffer,16);debug("\n");
    memset(frame_buffer, 0x0, TEXT_MODE_MEM_SIZE);

    flip_cursor(device);
}

static void do_scroll(ScreenDevice* device){
    flip_cursor(device);

    void* frame_buffer = device->frame_buffer;

    memcpy(frame_buffer, frame_buffer + LINESIZE * 4, TEXT_MODE_MEM_SIZE - LINESIZE * 4);
    
    memset(frame_buffer + TEXT_MODE_MEM_SIZE - LINESIZE * 4, 0 , LINESIZE * 4);

    flip_cursor(device);
}

static void set_mode(ScreenDevice* device, ModeSetting* mode_setting){

    SCREEN_DEVICE(device)->mode = mode_setting->mode;

    if (mode_setting->mode == MODE_TEXT){
        bga_set_text();
    } else {
        bga_set_mode(
            mode_setting->width, 
            mode_setting->height, 
            mode_setting->bpp
        );
    }
}

