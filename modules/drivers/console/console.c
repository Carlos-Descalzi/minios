#define NODEBUG
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
    CharDevice device;
    CharDevice* screen;
    CharDevice* keyboard;
    IORequest* user_request;
    IORequest console_request;
    uint8_t buffer[2];
    KeyboardStatus keyboard_status;
    ConsoleStatus console_status;
} ConsoleDevice;

#define CONSOLE_DEVICE(d)               ((ConsoleDevice*)d)

static uint8_t  count_devices           (DeviceType* device_type);
static Device*  instantiate             (DeviceType* device_type, uint8_t device_number);
static void     release                 (DeviceType* device_type, Device* device);
static void     request_callback        (IORequest* request, void* data);

static int16_t  console_write           (CharDevice* device, uint8_t data);
static int16_t  console_read_async      (CharDevice* device, IORequest* request);
static void     reset_console_request   (ConsoleDevice* device);
static void     set_keyboard_request    (ConsoleDevice* device);

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
    ConsoleDevice* device = heap_alloc(sizeof(ConsoleDevice));
    memset(device,0,sizeof(ConsoleDevice));
    DEVICE(device)->type = DEVICE_TYPE_CHAR;
    device->screen = CHAR_DEVICE(device_find(VIDEO,0));
    device->keyboard = CHAR_DEVICE(device_find(KBD,0));
    device->console_request.callback = request_callback;
    device->console_request.target_buffer = device->buffer;
    device->console_request.kernel = 1;
    device->keyboard_status.byte = 0;
    device->console_status.byte = 1;
    reset_console_request(device);
    CHAR_DEVICE(device)->read_async = console_read_async;
    CHAR_DEVICE(device)->write = console_write;
    DEVICE(device)->async = 1;
    return DEVICE(device);
}

static uint8_t count_devices(struct DeviceType* device_type){
    return 1;
}

static int16_t console_write(CharDevice* device, uint8_t data){
    return char_device_write(CONSOLE_DEVICE(device)->screen,data);
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
                char_device_write(CONSOLE_DEVICE(device)->screen,data);//buffer[0]);
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
