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
 * QEMU - VGA driver.
 * Disclaimer: I had problems to get text mode coexist with
 * graphics mode. So is all graphics mode even for text.
 * To make things worse I had some issues with linear frame buffer,
 * so for text mode I'm using banked framebuffer.
 * TODO: fix that crap.
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

#define VBE_DISPI_INDEX_BANK        0x05
#define VBE_DISPI_INDEX_VIRT_WIDTH  0x06
#define VBE_DISPI_INDEX_VIRT_HEIGHT 0x07
#define VBE_DISPI_INDEX_X_OFFSET    0x08
#define VBE_DISPI_INDEX_Y_OFFSET    0x09

#define VBE_DISPI_DISABLED          0x00
#define VBE_DISPI_ENABLED           0x01
#define VBE_DISPI_LFB_ENABLE        0x40
#define VBE_DISPI_NOCLEARMEM        0x80

#define MODE_TEXT                   0
#define MODE_GRAPHIC                1

#define TEXT_MODE_XRES              640
#define TEXT_MODE_YRES              400
#define TEXT_MODE_BPP               8
#define TEXT_MODE_CHAR_WIDTH        8
#define TEXT_MODE_CHAR_HEIGHT       16
#define TEXT_MODE_COLS              (TEXT_MODE_XRES / TEXT_MODE_CHAR_WIDTH)
#define TEXT_MODE_ROWS              (TEXT_MODE_YRES / TEXT_MODE_CHAR_HEIGHT)
#define TEXT_MODE_SCREEN_SIZE       (TEXT_MODE_XRES * TEXT_MODE_YRES)
#define TEXT_ROWSIZE                (TEXT_MODE_XRES * TEXT_MODE_CHAR_HEIGHT)
#define TEXT_MODE_FRAMEBUFFER       ( (uint8_t*) 0xA0000)
#define TEXT_MODE_BANK_SIZE         65536

static const uint32_t palette[] = {
    0x00000000,
    0x00770000,
    0x00007700,
    0x00777700,
    0x00000077,
    0x00770077,
    0x00007777,
    0x00777777,
    0x00000000,	
    0x00FF0000,
    0x0000FF00,
    0x00FFFF00,
    0x000000FF,
    0x00FF00FF,
    0x0000FFFF,
    0x00FFFFFF
};

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
static void     bga_set_mode        (uint16_t width, uint16_t height, uint8_t bpp, int linear);
static void     bga_set_text        (void);
static void     write_char          (ScreenDevice* device, uint8_t chr);
static void     flip_cursor         (ScreenDevice* device);
static void     clear_screen        (ScreenDevice* device);
static void     do_scroll           (ScreenDevice* device);
static void     set_mode            (ScreenDevice* device, ModeSetting* mode_setting);
static void     set_bank            (int bank);

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

    for (int i=0;i<16;i++){
        outb(0x3C8, i);
        for (int j=0;j<3;j++){
            outb(0x3c9, (palette[i] >> (8 * j)) & 0xFF);
        }
    }

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


static uint32_t get_pos(ScreenDevice* device){
    uint16_t x_pos = (device->pos % TEXT_MODE_COLS) ;
    uint16_t y_pos = (device->pos / TEXT_MODE_COLS) ;

    return y_pos * (TEXT_MODE_XRES * TEXT_MODE_CHAR_HEIGHT) + x_pos * TEXT_MODE_CHAR_WIDTH;
}

static void write_char(ScreenDevice* device, uint8_t val){

    device->input_buffer[device->buff_index++] = val;

    if (device->buff_index == 2){
        flip_cursor(device);

        uint8_t color = device->input_buffer[0];
        uint8_t chr = device->input_buffer[1];

        const uint8_t* chr_font = VGA_FONT_16 + chr * TEXT_MODE_CHAR_HEIGHT;

        uint32_t pos = get_pos(device);

        uint32_t fg = color & 0xF;
        uint32_t bg = color >> 4;

        for (int i=0;i<TEXT_MODE_CHAR_HEIGHT;i++){
            uint32_t p = pos + i * TEXT_MODE_XRES;
            set_bank(p >> 16);
            p &= 0xFFFF;
            uint8_t row = chr_font[i];
            for (int j=0;j<TEXT_MODE_CHAR_WIDTH;j++){
                TEXT_MODE_FRAMEBUFFER[p+j] = (row & (1 << (7-j))) ? fg : bg; 
            }
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

    uint32_t pos = get_pos(device);

    for (int i=0;i<TEXT_MODE_CHAR_HEIGHT;i++){
        uint32_t p = pos + i * TEXT_MODE_XRES;
        set_bank(p >> 16);
        p &= 0xFFFF;
        for (int j=0;j<TEXT_MODE_CHAR_WIDTH;j++){
            TEXT_MODE_FRAMEBUFFER[p+j] ^= 7;
        }
    }

}

static void clear_screen(ScreenDevice* device){

    int banks = (TEXT_MODE_SCREEN_SIZE >> 16) 
        + (TEXT_MODE_SCREEN_SIZE & 0xFFFF ? 1 : 0);

    for (int i=0;i<banks;i++){
        set_bank(i);
        memsetdw(TEXT_MODE_FRAMEBUFFER, 0x00, TEXT_MODE_BANK_SIZE / sizeof(uint32_t));
    }

    device->pos = 0;

    flip_cursor(device);
}

static void do_scroll(ScreenDevice* device){

    flip_cursor(device);

    uint8_t row[TEXT_MODE_XRES];

    for (int i=0;i<24*TEXT_MODE_CHAR_HEIGHT;i++){

        uint32_t rowindext = i * TEXT_MODE_XRES;
        uint32_t rowindexs = rowindext + TEXT_ROWSIZE;

        set_bank(rowindexs >> 16);
        memcpydw(row, TEXT_MODE_FRAMEBUFFER + (rowindexs & 0xFFFF), TEXT_MODE_XRES / sizeof(uint32_t));

        set_bank(rowindext >> 16);
        memcpydw(TEXT_MODE_FRAMEBUFFER + (rowindext & 0xFFFF), row, TEXT_MODE_XRES / sizeof(uint32_t));

    }
    for (int i=0;i<TEXT_MODE_CHAR_HEIGHT;i++){
        uint32_t rowindex = (TEXT_MODE_YRES - TEXT_MODE_CHAR_HEIGHT + i ) * TEXT_MODE_XRES;

        set_bank(rowindex >> 16);
        memsetdw(TEXT_MODE_FRAMEBUFFER + (rowindex & 0xFFFF), 0, TEXT_MODE_XRES / sizeof(uint32_t));
    }

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
            mode_setting->bpp,
            1
        );
    }
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
    bga_set_mode(TEXT_MODE_XRES, TEXT_MODE_YRES, TEXT_MODE_BPP, 0);
}

static void bga_set_mode(uint16_t width, uint16_t height, uint8_t bpp, int linear){
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
            | (linear ? VBE_DISPI_LFB_ENABLE : 0)
    );
}

static void set_bank(int bank){
    static int _bank = 0;
    if (_bank != bank){
        bga_write(VBE_DISPI_INDEX_BANK, bank);
        _bank = bank;
    }
}
