//#define NODEBUG
#include "kernel/device.h"
#include "lib/heap.h"
#include "misc/debug.h"
#include "lib/string.h"
#include "kernel/task.h"
#include "misc/keycodes.h"

typedef union {
    struct {
        uint8_t shift:1,
                alt:1,
                ctrl:1,
                caps:1,
                num:1,
                scroll:1;
    };
    uint8_t byte;
} KeyboardStatus;

typedef union {
    struct {
        uint8_t echo:1;

    };
    uint8_t byte;
} ConsoleStatus;

typedef struct {
    uint8_t x;
    uint8_t y;
} Cursor;

typedef struct {
    CharDevice device;
    CharDevice* screen;
    CharDevice* keyboard;
    uint16_t* screen_address; 
    KeyboardStatus keyboard_status;
    ConsoleStatus console_status;
    Cursor pos;
    Cursor saved_pos;
    uint8_t color;
    uint8_t mode;
    IORequest* user_request;
    IORequest console_request;
    uint8_t buffer[2];
    uint8_t buff_index;
    char escape_buff[8];
    uint8_t width;
    uint8_t height;

} ConsoleDevice;

#define MODE_TEXT                       0
#define MODE_ESCAPE                     1

#define OPT_CURSOR_ON                   1
#define OP_TCURSOR_OFF                  2
#define OPT_CURSOR_POS                  3

#define CONSOLE_DEVICE(d)               ((ConsoleDevice*)d)

static uint8_t  count_devices           (DeviceType* device_type);
static Device*  instantiate             (DeviceType* device_type, uint8_t device_number);
static void     release                 (DeviceType* device_type, Device* device);
static void     request_callback        (IORequest* request, void* data);

static int16_t  console_write           (CharDevice* device, uint8_t data);
static void     handle_console_write    (ConsoleDevice* device, uint8_t data);
static int16_t  console_read_async      (CharDevice* device, IORequest* request);
static void     reset_console_request   (ConsoleDevice* device);
static void     set_keyboard_request    (ConsoleDevice* device);
uint8_t         handle_escape_char      (ConsoleDevice* screen, uint8_t chr);
static void     move_cursor_up          (ConsoleDevice* screen);
static void     move_cursor_dn          (ConsoleDevice* screen);
static void     move_cursor_right       (ConsoleDevice* screen);
static void     move_cursor_left        (ConsoleDevice* screen);
static void     move_cursor             (ConsoleDevice* screen);
static void     save_cursor             (ConsoleDevice* screen);
static void     restore_cursor          (ConsoleDevice* screen);
static void     set_color               (ConsoleDevice* screen);
static void     backspace               (ConsoleDevice* screen);
static void     tab                     (ConsoleDevice* screen);
static void     newline                 (ConsoleDevice* screen);
static void     clear_buff              (ConsoleDevice* screen);
static void     update_cursor           (ConsoleDevice* console);
static void     clear_screen            (ConsoleDevice* console);
static void     fb_write                (ConsoleDevice* screen, uint8_t chr);
static void     adjust_view_to_pos      (ConsoleDevice* console);

static DeviceType DEVICE_TYPE;

void module_init(){
    DEVICE_TYPE.kind = TERM;
    DEVICE_TYPE.count_devices = count_devices;
    DEVICE_TYPE.instantiate = instantiate;
    DEVICE_TYPE.release = release;
    device_register_type((DeviceType*)&DEVICE_TYPE);
    debug("** Console device type registered\n");
}

static void release(DeviceType* device_type, Device* device){
    heap_free(device);
}

static void reset_console_request(ConsoleDevice* device){
    device->console_request.dsize = 0;
    device->console_request.size = 2;
    device->console_request.kernel = 1;
    device->user_request = NULL;
    device->console_request.callback_data = device;
}

static Device* instantiate(DeviceType* device_type, uint8_t device_number){
    CharDevice* screen = CHAR_DEVICE(device_find(VIDEO,0));
    
    if (!screen){
        debug("Unable to setup console, no screen\n");
    }

    CharDevice* keyboard = CHAR_DEVICE(device_find(KBD,0));

    if (!keyboard){
        debug("Unable to setup console, no keyboard\n");
    }

    ConsoleDevice* device = heap_alloc(sizeof(ConsoleDevice));
    memset(device,0,sizeof(ConsoleDevice));

    DEVICE(device)->type = DEVICE_TYPE_CHAR;
    DEVICE(device)->kind = TERM;
    DEVICE(device)->async = 1;
    DEVICE(device)->mmapped = 0;

    CHAR_DEVICE(device)->read_async = console_read_async;
    CHAR_DEVICE(device)->write = console_write;

    device->screen = screen;
    device->keyboard = keyboard;
    device->screen_address = (uint16_t*) device_base_address(screen);

    device_setopt(device->screen, OPT_CURSOR_ON, NULL);

    device->console_request.callback = request_callback;
    device->console_request.target_buffer = device->buffer;
    device->console_request.kernel = 1;
    device->keyboard_status.byte = 0;
    device->console_status.byte = 1;
    device->mode = MODE_TEXT;
    device->pos.x = 0;
    device->pos.y = 0;
    device->saved_pos.x = 0;
    device->saved_pos.y = 0;
    device->color = 15;
    device->width = 80;
    device->height = 25;

    reset_console_request(device);

    clear_screen(device);

    return DEVICE(device);
}

static uint8_t count_devices(struct DeviceType* device_type){
    return 1;
}

static int16_t console_write(CharDevice* device, uint8_t data){
    handle_console_write(CONSOLE_DEVICE(device),data);
    return 0;
}

static int16_t  console_read_async  (CharDevice* device, IORequest* request){
    CONSOLE_DEVICE(device)->user_request = request;
    return char_device_read_async(
        CONSOLE_DEVICE(device)->keyboard, 
        &(CONSOLE_DEVICE(device)->console_request)
    );
}

static uint8_t encode(ConsoleDevice* device, uint16_t code){
    if (code >= KEY_CODE_A && code <= KEY_CODE_Z){
        if (device->keyboard_status.shift
            || device->keyboard_status.caps){
            return code & 0xFF;
        } else {
            return (code - KEY_CODE_A) + 'a';
        }
    } else if (code >= KEY_CODE_0 && code <= KEY_CODE_9){
        if (device->keyboard_status.shift){
            switch(code){
                case KEY_CODE_0:
                    return '=';
                case KEY_CODE_1:
                    return '!';
                case KEY_CODE_2:
                    return '"';
                case KEY_CODE_3:
                    return '#';
                case KEY_CODE_4:
                    return '$';
                case KEY_CODE_5:
                    return '%';
                case KEY_CODE_6:
                    return '&';
                case KEY_CODE_7:
                    return '/';
                case KEY_CODE_8:
                    return '(';
                case KEY_CODE_9:
                    return ')';
            }
        }
    } else if (code == KEY_CODE_COMMA){
        if (device->keyboard_status.shift){
            return ';';
        } 
        return ',';
    } else if (code == KEY_CODE_PERIOD){
        if (device->keyboard_status.shift){
            return ':';
        }
        return '.';
    }
    return code & 0xFF;
}

static void request_callback(IORequest* request, void* data){
    ConsoleDevice* device = data;

    uint16_t key_code = (*((uint16_t*)device->console_request.target_buffer));

    if (key_code & 0x8000){
        key_code &= 0x7FFF;

        if (key_code == KEY_CODE_LEFT_SHIFT
            || key_code == KEY_CODE_RIGHT_SHIFT){
            device->keyboard_status.shift = 1;
            set_keyboard_request(device);
        } else if (key_code == KEY_CODE_LEFT_ALT){
            device->keyboard_status.alt = 1;
            set_keyboard_request(device);
        } else if (key_code == KEY_CODE_LEFT_CTRL){
            device->keyboard_status.ctrl = 1;
            set_keyboard_request(device);
        } else if (key_code == KEY_CODE_CAPS_LOCK){
            device->keyboard_status.caps = ~device->keyboard_status.caps;
            set_keyboard_request(device);
        } else if (key_code == KEY_CODE_NUMLOCK){
            device->keyboard_status.num = ~device->keyboard_status.num;
            set_keyboard_request(device);
        } else if (
            (key_code >= KEY_CODE_0 && key_code <= KEY_CODE_Z)
            || (key_code >= KEY_CODE_KP_0 && key_code <= KEY_CODE_KP_PERIOD)
            || (key_code == KEY_CODE_ESCAPE)
            || (key_code == KEY_CODE_ENTER)
            || (key_code == KEY_CODE_BS)
            || (key_code == KEY_CODE_TAB)
            || (key_code == KEY_CODE_SPACE)
            || (key_code >= KEY_CODE_MINUS && key_code <= KEY_CODE_BACK_TICK)) {
            char data = encode(device, key_code);

            if (device->console_status.echo){
                handle_console_write(CONSOLE_DEVICE(device),data);
            }
            handle_io_request(device->user_request, &data, 1, TASK_IO_REQUEST_DONE);
            reset_console_request(device);
        } else {
            debug("Unknown keycode\n");
        }
    } else {
        if (key_code == KEY_CODE_LEFT_SHIFT
            || key_code == KEY_CODE_RIGHT_SHIFT){
            device->keyboard_status.shift = 0;
        } else if (key_code == KEY_CODE_LEFT_ALT){
            device->keyboard_status.alt = 0;
        } else if (key_code == KEY_CODE_LEFT_CTRL){
            device->keyboard_status.ctrl = 0;
        }
        set_keyboard_request(device);
    }
}

static void set_keyboard_request(ConsoleDevice* device){
    device->console_request.size = 2;
    device->console_request.dsize = 0;
    memset(device->console_request.target_buffer,0,2);
    char_device_read_async(
        CONSOLE_DEVICE(device)->keyboard,
        &(CONSOLE_DEVICE(device)->console_request)
    );
}
static void handle_console_write(ConsoleDevice* device, uint8_t chr){
    if (device->mode == MODE_ESCAPE){
        if (chr != '[' && device->buff_index == 0){
            // exit escape mode, just send the esc character
            device->mode = MODE_TEXT;
            fb_write(device, 27);
            fb_write(device, chr);
        } else {
            if (handle_escape_char(device, chr)){
                device->mode = MODE_TEXT;
            }
        }
    } else {
        if (chr == 27){
            clear_buff(device);
            device->mode = MODE_ESCAPE;
        } else if (chr == 8){
            backspace(device);
        } else if (chr == 9){
            tab(device);
        } else if (chr == '\n'){
            newline(device);
        } else {
            fb_write(device, chr);
        }
    }
}

uint8_t handle_escape_char(ConsoleDevice* device, uint8_t chr){
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
            clear_screen(device);
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
            device->escape_buff[device->buff_index++] = chr;
            break;
    }
    return 0;
}
static void move_cursor_up(ConsoleDevice* console){
    uint8_t dy;

    if (strlen(console->escape_buff)){
        dy = atoi(console->escape_buff);
    } else {
        dy = 1;
    }

    if (dy > console->pos.y){
        console->pos.y = 0;
    } else {
        console->pos.y-=dy;
    }

    update_cursor(console);
}

static void move_cursor_dn(ConsoleDevice* console){
    uint8_t dy;

    if (strlen(console->escape_buff)){
        dy = atoi(console->escape_buff);
    } else {
        dy = 1;
    }

    console->pos.y+=dy;

    if (console->pos.y > 24){
        console->pos.y = 24;
    }

    update_cursor(console);
}

static void move_cursor_right(ConsoleDevice* console){
    uint8_t dx;

    dx = atoi(console->escape_buff+1);

    console->pos.x+=dx;
    if (console->pos.x > 79){
        console->pos.x = 79;
    }
    update_cursor(console);
}

static void move_cursor_left(ConsoleDevice* console){
    uint8_t dx;

    dx = atoi(console->escape_buff+1);

    if (dx < console->pos.x){
        console->pos.x = 0;
    } else {
        console->pos.x-= dx;
    }
    update_cursor(console);
}

static void move_cursor(ConsoleDevice* console){
    char* c;

    c = strrchr(console->escape_buff,';');
    if (c){
        console->pos.x = atoi(c+1);
        *c = '\0';
        console->pos.y = atoi(console->escape_buff+1);
        update_cursor(console);
    }
}

static void tab(ConsoleDevice* console){

    uint8_t new_x = console->pos.x / 8 + 8;
    if (new_x <= console->pos.x){new_x+=8;}

    console->pos.x = new_x;
    update_cursor(console);
}

static void backspace(ConsoleDevice* console){

    if (console->pos.x > 0){
        console->pos.x--;
        fb_write(console, ' ');
        console->pos.x--;
        update_cursor(console);
    }
}
static void newline(ConsoleDevice* console){
    console->pos.x = 0;
    console->pos.y++;
    adjust_view_to_pos(console);
    update_cursor(console);
}

static void update_cursor(ConsoleDevice* console){
    device_setopt(console->screen, OPT_CURSOR_POS, &(console->pos));
}

static void save_cursor(ConsoleDevice* console){
    console->saved_pos = console->pos;
    update_cursor(console);
}

static void restore_cursor(ConsoleDevice* console){
    console->pos = console->saved_pos;
    update_cursor(console);
}

static void set_color(ConsoleDevice* screen){
    int bg;
    int fg;
    char* c;

    c = strrchr(screen->escape_buff,';');

    if (c){
        if (strlen(c+1)){
            bg = atoi(c+1);
        }
        *c='\0';
        c = strrchr(screen->escape_buff,'[');
        if (strlen(c+1)){
            fg = atoi(c+1);
        }
        *c='\0';
        //b = atoi(buff+1); TODO: intensity
    }

    screen->color = bg << 4 | fg;
}

static inline void clear_buff(ConsoleDevice* screen){
    memset(screen->escape_buff,0,8);
    screen->buff_index = 0;
}
static void clear_screen (ConsoleDevice* console){
    int screen_size = console->width * console->height;
    for (int i=0;i<screen_size;i++){
        console->screen_address[i] = console->color << 8;
    }
    console->pos.x = 0;
    console->pos.y = 0;
    update_cursor(console);
}
void fb_write (ConsoleDevice* console, uint8_t chr){

    uint16_t pos = console->pos.y * console->width + console->pos.x;

    console->screen_address[pos] = (console->color << 8) | chr;

    console->pos.x++;
    adjust_view_to_pos(console);
    update_cursor(console);
}

static void adjust_view_to_pos(ConsoleDevice* console){

    int screen_size = console->width * console->height;

    if (console->pos.x >= 80){
        console->pos.x = 0;
        console->pos.y++;
    }

    if (console->pos.y >= 25){
        console->pos.y--;

        for (int i=0;i<(screen_size - console->width);i++){
            console->screen_address[i] = console->screen_address[i+console->width];
        }
        for (int i=0;i<console->width;i++){
            console->screen_address[(screen_size - console->width)+i] = console->color << 8;
        }

    }
}
