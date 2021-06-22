//#define NODEBUG
#include "screen.h"
#include "kernel/device.h"
#include "board/console.h"
#include "lib/string.h"
#include "lib/stdlib.h"
#include "misc/debug.h"
#include "lib/heap.h"

/**
 * This device is just a wrapper over raw console API.
 * adding basic ANSI terminal support.
 **/

#define DEVICE_SUBTYPE_SCREEN    1

#define SCREEN_OPT_CURSOR_ON    1
#define SCREEN_OPT_CURSOR_OFF   2

#define MODE_TEXT   0
#define MODE_ESCAPE 1

typedef struct {
    CharDevice device;
    uint8_t mode;
    uint8_t buff_index;
    char buff[8];
    uint8_t saved_x;
    uint8_t saved_y;
} ScreenDevice;

#define SCREEN_DEVICE(d)    ((ScreenDevice*)d)

static int16_t  screen_setopt       (Device* device, uint32_t option, void* data);
static int16_t  screen_read         (CharDevice* device);
static int16_t  screen_write        (CharDevice* device, uint8_t chr);
uint8_t         handle_escape_char  (ScreenDevice* screen, uint8_t chr);
static void     move_cursor_up      (ScreenDevice* screen);
static void     move_cursor_dn      (ScreenDevice* screen);
static void     move_cursor_right   (ScreenDevice* screen);
static void     move_cursor_left    (ScreenDevice* screen);
static void     move_cursor         (ScreenDevice* screen);
static void     save_cursor         (ScreenDevice* screen);
static void     restore_cursor      (ScreenDevice* screen);
static void     set_color           (ScreenDevice* screen);
static uint8_t  count_devices       (DeviceType* device_type);
static Device*  instantiate         (DeviceType* device_type, uint8_t device_number);
static void     release             (DeviceType* device_type, Device* device);
static void     clear_buff          (ScreenDevice* screen);

static DeviceType SCREEN_DEVICE_TYPE = {
    kind: VIDEO,
    count_devices: count_devices,
    instantiate: instantiate,
    release: release
};


static uint8_t count_devices(DeviceType* device_type){
    return 1;
}

static Device* instantiate(DeviceType* device_type, uint8_t device_number){
    console_init();
    ScreenDevice* device = heap_alloc(sizeof(ScreenDevice));
    
    device->mode = MODE_TEXT;
    device->saved_x = 0;
    device->saved_y = 0;
    device->device.read = screen_read;
    device->device.write = screen_write;
    device->device.base.setopt = screen_setopt;
    return DEVICE(device);
}

static void release(DeviceType* device_type, Device* device){
    heap_free(device);
}

void screen_register(){
    device_register_type((DeviceType*)&SCREEN_DEVICE_TYPE);
}


static int16_t screen_setopt(Device* device, uint32_t option, void* data){
    switch(option){
        case SCREEN_OPT_CURSOR_ON:
            console_cursor_on();
            return 0;
        case SCREEN_OPT_CURSOR_OFF:
            console_cursor_off();
            return 0;
    }
    return -1;
}

static int16_t screen_read(CharDevice* device){
    return 0;
}

static int16_t screen_write(CharDevice* device, uint8_t chr){
    ScreenDevice* screen = SCREEN_DEVICE(device);

    if (screen->mode == MODE_ESCAPE){
        debug_c(chr);debug(" ");debug_i(screen->buff_index,10);debug("\n");
        if (chr != '[' && screen->buff_index == 0){
            // exit escape mode, just send the esc character
            debug("exit escape mode\n");
            screen->mode = MODE_TEXT;
            console_put(27);
            console_put(chr);
        } else {
            debug("add to buffer\n");
            if (handle_escape_char(screen, chr)){
                screen->mode = MODE_TEXT;
            }
        }
    } else {
        if (chr == 27){
            debug("escape mode\n");
            clear_buff(screen);
            screen->mode = MODE_ESCAPE;
        } else {
            console_put(chr);
        }
    }
    return 0;
}


uint8_t handle_escape_char(ScreenDevice* device, uint8_t chr){
    switch (chr){
        case 'A':
            move_cursor_up(device);
            return 1;
        case 'B':
            move_cursor_dn(device);
            return 1;
        case 'C':
            move_cursor_right(device);
            return 1;
        case 'D':
            move_cursor_left(device);
            return 1;
        case 'f':
            move_cursor(device);
            return 1;
        case 'J':
            debug("clear screen\n");
            console_clear_screen();
            console_gotoxy(0,0);
            return 1;
        case 'K':
            return 1;
        case 'p':
            return 1;
        case 's':
            save_cursor(device);
            return 1;
        case 'u':
            restore_cursor(device);
            return 1;
        case 'm':
            set_color(device);
            return 1;
        case '[':
        default:
            debug("added\n");
            device->buff[device->buff_index++] = chr;
            break;
    }
    return 0;
}

static void move_cursor_up(ScreenDevice* screen){
    uint8_t x;
    uint8_t y;
    uint8_t dy;
    console_get_cursor_pos(&x,&y);

    if (strlen(screen->buff)){
        dy = atoi(screen->buff);
    } else {
        dy = 1;
    }

    if (dy > y){
        y = 0;
    } else {
        y-=dy;
    }
    console_gotoxy(x,y);
}
static void move_cursor_dn(ScreenDevice* screen){
    uint8_t x;
    uint8_t y;
    uint8_t dy;
    console_get_cursor_pos(&x,&y);

    if (strlen(screen->buff)){
        dy = atoi(screen->buff);
    } else {
        dy = 1;
    }

    y+=dy;

    if (y > 24){
        y = 24;
    }
    console_gotoxy(x,y);
}

static void move_cursor_right(ScreenDevice* screen){
    uint8_t x;
    uint8_t y;
    uint8_t dx;
    console_get_cursor_pos(&x,&y);
    dx = atoi(screen->buff+1);

    x+=dx;
    if (x > 79){
        x = 79;
    }
    console_gotoxy(x,y);
}

static void move_cursor_left(ScreenDevice* screen){
    uint8_t x;
    uint8_t y;
    uint8_t dx;
    console_get_cursor_pos(&x,&y);
    dx = atoi(screen->buff+1);
    if (dx < x){
        x = 0;
    } else {
        x-= dx;
    }
    console_gotoxy(x, y);
}

static void move_cursor(ScreenDevice* screen){
    int x;
    int y;
    char* c;

    c = strrchr(screen->buff,';');
    if (c){
        x = atoi(c+1);
        *c = '\0';
        y = atoi(screen->buff+1);
        console_gotoxy(x,y);
    }
}

static void save_cursor(ScreenDevice* screen){
    console_get_cursor_pos(&(screen->saved_x),&(screen->saved_y));
}

static void restore_cursor(ScreenDevice* screen){
    console_gotoxy(screen->saved_x, screen->saved_y);
}

static void set_color(ScreenDevice* screen){
    int bg;
    int fg;
    //int b;
    char* c;

    c = strrchr(screen->buff,';');

    if (c){
        if (strlen(c+1)){
            bg = atoi(c+1);
        }
        *c='\0';
        c = strrchr(screen->buff,';');
        if (strlen(c+1)){
            fg = atoi(c+1);
        }
        *c='\0';
        //b = atoi(buff+1); TODO: intensity
    }

    console_color(bg << 4 | fg); // TODO: Intensity
}

static inline void clear_buff(ScreenDevice* screen){
    memset(screen->buff,0,8);
    screen->buff_index = 0;
}
