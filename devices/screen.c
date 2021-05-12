#include "device.h"
#include "screen.h"
#include "string.h"
#include "console.h"
#include "stdlib.h"
#include "debug.h"

/**
 * This device is just a wrapper over raw console API.
 * adding basic ANSI terminal support.
 **/

#define DEVICE_SUBTYPE_SCREEN    1

#define SCREEN_OPT_CURSOR_ON    1
#define SCREEN_OPT_CURSOR_OFF   2

#define MODE_TEXT   0
#define MODE_ESCAPE 1

static int16_t  screen_setopt       (Device* device, uint32_t option, void* data);
static int16_t  screen_read         (CharDevice* device);
static int16_t  screen_write        (CharDevice* device, uint8_t chr);
uint8_t         handle_escape_char  (uint8_t chr);
static void     move_cursor_up      (void);
static void     move_cursor_dn      (void);
static void     move_cursor_right   (void);
static void     move_cursor_left    (void);
static void     move_cursor         (void);
static void     save_cursor         (void);
static void     restore_cursor      (void);
static void     set_color           (void);
static uint8_t  count_devices       (struct DeviceType* device_type);
static Device*  instantiate         (struct DeviceType* device_type, uint8_t device_number);
static void     release             (struct DeviceType* device_type, Device* device);
static void     clear_buff          (void);

static uint8_t mode;
static uint8_t buff_index;
static char buff[8];
static uint8_t saved_x;
static uint8_t saved_y;
static CharDevice SCREEN_DEVICE = {
    base: {
        type: DEVICE_TYPE_CHAR,
        //subtype: DEVICE_SUBTYPE_SCREEN,
        setopt: screen_setopt,
    },
    read: screen_read,
    write: screen_write
};

static DeviceType SCREEN_DEVICE_TYPE = {
    kind: CON,
    count_devices: count_devices,
    instantiate: instantiate,
    release: release
};

static uint8_t count_devices(struct DeviceType* device_type){
    return 1;
}

static Device* instantiate(struct DeviceType* device_type, uint8_t device_number){
    console_init();
    clear_buff();
    mode = MODE_TEXT;
    saved_x = 0;
    saved_y = 0;
    console_print("Screen device initialized\n");
    return DEVICE(&SCREEN_DEVICE);
}

static void release(struct DeviceType* device_type, Device* device){
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
    if (mode == MODE_ESCAPE){
        if (chr != ']'){
            // exit escape mode, just send the esc character
            mode = MODE_TEXT;
            console_put(27);
            console_put(chr);
        } else {
            if (handle_escape_char(chr)){
                mode = MODE_TEXT;
            }
        }
    } else {
        if (chr == 27){
            clear_buff();
            mode = MODE_ESCAPE;
        } else {
            console_put(chr);
        }
    }
}


uint8_t handle_escape_char(uint8_t chr){
    switch (chr){
        case 'A':
            move_cursor_up();
            return 1;
        case 'B':
            move_cursor_dn();
            return 1;
        case 'C':
            move_cursor_right();
            return 1;
        case 'D':
            move_cursor_left();
            return 1;
        case 'f':
            move_cursor();
            return 1;
        case 'J':
            console_clear_screen();
            return 1;
        case 'K':
            return 1;
        case 'p':
            return 1;
        case 's':
            save_cursor();
            return 1;
        case 'u':
            restore_cursor();
            return 1;
        case 'm':
            set_color();
            return 1;
        default:
            buff[buff_index++] = chr;
            break;
    }
    return 0;
}

static void move_cursor_up(){
    uint8_t x;
    uint8_t y;
    uint8_t dy;
    console_get_cursor_pos(&x,&y);

    if (strlen(buff)){
        dy = atoi(buff);
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
static void move_cursor_dn(){
    uint8_t x;
    uint8_t y;
    uint8_t dy;
    console_get_cursor_pos(&x,&y);

    if (strlen(buff)){
        dy = atoi(buff);
    } else {
        dy = 1;
    }

    y+=dy;

    if (y > 24){
        y = 24;
    }
    console_gotoxy(x,y);
}

static void move_cursor_right(){
    uint8_t x;
    uint8_t y;
    uint8_t dx;
    console_get_cursor_pos(&x,&y);
    dx = atoi(buff+1);

    x+=dx;
    if (x > 79){
        x = 79;
    }
    console_gotoxy(x,y);
}

static void move_cursor_left(){
    uint8_t x;
    uint8_t y;
    uint8_t dx;
    console_get_cursor_pos(&x,&y);
    dx = atoi(buff+1);
    if (dx < x){
        x = 0;
    } else {
        x-= dx;
    }
    console_gotoxy(x, y);
}

static void move_cursor(){
    int x;
    int y;
    char* c;

    c = strrchr(buff,';');
    if (c){
        x = atoi(c+1);
        *c = '\0';
        y = atoi(buff+1);
        console_gotoxy(x,y);
    }
}

static void save_cursor(){
    console_get_cursor_pos(&saved_x,&saved_y);
}

static void restore_cursor(){
    console_gotoxy(saved_x, saved_y);
}

static void set_color(){
    int bg;
    int fg;
    int b;
    char* c;

    c = strrchr(buff,';');

    if (c){
        if (strlen(c+1)){
            bg = atoi(c+1);
        }
        *c='\0';
        c = strrchr(buff,';');
        if (strlen(c+1)){
            fg = atoi(c+1);
        }
        *c='\0';
        b = atoi(buff+1);
    }

    console_color(bg << 4 | fg); // TODO: Intensity
}

static inline void clear_buff(){
    memset(buff,0,8);
    buff_index = 0;
}
