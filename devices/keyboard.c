#define NODEBUG
#include "lib/stddef.h"
#include "keyboard.h"
#include "kernel/device.h"
#include "kernel/isr.h"
#include "board/ps2.h"
#include "board/pic.h"
#include "lib/heap.h"
#include "kernel/spinlock.h"
#include "kernel/isr.h"
#include "misc/debug.h"
#include "misc/keycodes.h"

static const uint8_t    KEY_CODES[] = {
    0,                  KEY_CODE_ESCAPE,        KEY_CODE_1,             KEY_CODE_2,             
    KEY_CODE_3,         KEY_CODE_4,             KEY_CODE_5,             KEY_CODE_6,             
    KEY_CODE_7,         KEY_CODE_8,             KEY_CODE_9,             KEY_CODE_0,             
    KEY_CODE_MINUS,     KEY_CODE_EQUAL,         KEY_CODE_BS,            KEY_CODE_TAB,

    KEY_CODE_Q,         KEY_CODE_W,             KEY_CODE_E,             KEY_CODE_R,             
    KEY_CODE_T,         KEY_CODE_Y,             KEY_CODE_U,             KEY_CODE_I,             
    KEY_CODE_O,         KEY_CODE_P,             KEY_CODE_OPEN_BR,       KEY_CODE_CLOSE_BR,      
    KEY_CODE_ENTER,     KEY_CODE_LEFT_CTRL,     KEY_CODE_A,             KEY_CODE_S,

    KEY_CODE_D,         KEY_CODE_F,             KEY_CODE_G,             KEY_CODE_H,             
    KEY_CODE_J,         KEY_CODE_K,             KEY_CODE_L,             KEY_CODE_SEMICOLON,     
    KEY_CODE_SQUOTE,    KEY_CODE_BACK_TICK,     KEY_CODE_LEFT_SHIFT,    KEY_CODE_BACKSLASH,     
    KEY_CODE_Z,         KEY_CODE_X,             KEY_CODE_C,             KEY_CODE_V,             

    KEY_CODE_B,         KEY_CODE_N,             KEY_CODE_M,             KEY_CODE_COMMA,         
    KEY_CODE_PERIOD,    KEY_CODE_SLASH,         KEY_CODE_RIGHT_SHIFT,   KEY_CODE_KP_ASTERISK,   
    KEY_CODE_LEFT_ALT,  KEY_CODE_SPACE,         KEY_CODE_CAPS_LOCK,     KEY_CODE_F1,            
    KEY_CODE_F2,        KEY_CODE_F3,            KEY_CODE_F4,            KEY_CODE_F5,            

    KEY_CODE_F6,        KEY_CODE_F7,            KEY_CODE_F8,            KEY_CODE_F9,            
    KEY_CODE_NUMLOCK,   KEY_CODE_SCROLLOCK,     KEY_CODE_KP_7,          KEY_CODE_KP_8,          
    KEY_CODE_KP_9,      KEY_CODE_KP_4,          KEY_CODE_KP_5,          KEY_CODE_KP_6,          
    KEY_CODE_KP_PLUS,   KEY_CODE_KP_1,          KEY_CODE_KP_2,          KEY_CODE_KP_3,          

    KEY_CODE_KP_0,      KEY_CODE_KP_PERIOD,     0,                      0,
    0,                  KEY_CODE_F11,           KEY_CODE_F12
};

#define KEY_CODES_SIZE  (sizeof(KEY_CODES))


#define BUFFER_SIZE 256

#define KEY_A           0x1C
#define KEY_B           0x32
#define KEY_C           0x21
#define KEY_D           0x23
#define KEY_E           0x24
#define KEY_F           0x2B
#define KEY_G           0x34
#define KEY_H           0x33
#define KEY_I           0x43
#define KEY_J           0x3B
#define KEY_K           0x42

#define SCAN_BREAK      0xF0
#define SCAN_ALTCODE    0xE0

static uint8_t  count_devices       (DeviceType* device_type);
static Device*  instantiate         (DeviceType* device_type, uint8_t device_number);
static void     release             (DeviceType* device_type, Device* device);
static int16_t  setopt              (Device* device, uint32_t option, void* data);
static int16_t  read                (CharDevice* device);
static void     handle_keyboard_irq (InterruptFrame* frame, void* data);

typedef union {
    struct {
        uint16_t keycode:15,
                 press_release: 1;
    };
    uint16_t wdata;
    char cdata[2];
} KeyEvent;

typedef struct {
    CharDevice device;
    KeyEvent key_event;
    uint8_t pos;
    uint32_t ticks;
    Lock lock;
} KeyboardDevice;

#define KEYBOARD_DEVICE(d)  ((KeyboardDevice*)d)


static DeviceType DEVICE_TYPE = {
    kind: KBD,
    count_devices: count_devices,
    instantiate: instantiate,
    release: release
};

static KeyboardDevice KEYBOARD = {
    device: {
        base: {
            type: DEVICE_TYPE_CHAR,
            setopt: setopt,
        },
        read: read,
        write: NULL
    },
    key_event : { wdata : 0 },
    pos: 0,
    ticks: 0,
    lock: 0
};

void keyboard_register(){
    device_register_type((DeviceType*)&DEVICE_TYPE);
}

static uint8_t count_devices(DeviceType* device_type){
    return 1;
}
static Device* instantiate(DeviceType* device_type, uint8_t device_number){
    KEYBOARD.pos = 0;
    debug("Initiating keyboard driver\n");

    isr_install(PIC_IRQ_BASE + PS2_IRQ, handle_keyboard_irq, NULL);
    return DEVICE(&KEYBOARD);
}
static void release(DeviceType* device_type, Device* device){
    isr_install(PIC_IRQ_BASE + PS2_IRQ, NULL, NULL);
    release_lock(&(KEYBOARD.lock));
}
static int16_t setopt(Device* device, uint32_t option, void* data){
    return 0;
}

static int16_t read(CharDevice* device){
    int16_t val;
    if (KEYBOARD.pos == 1){
        KEYBOARD.pos = 0;
        return KEYBOARD.key_event.cdata[1];
    } 
    while (KEYBOARD.key_event.wdata == 0){
        acquire_lock(&(KEYBOARD.lock));
    }
    return KEYBOARD.key_event.cdata[KEYBOARD.pos++];
}

#define FLAG_ALTCODE    0x01
#define FLAG_BREAK      0x02

static uint16_t scan_code_to_key_code(uint8_t altcode, uint8_t scan_code){
    if (altcode){
        debug("Unknown ALT key code ");debug_i(scan_code,16);debug("\n");
        // TODO finish
        return 0;
    }
    if (scan_code > 0 && scan_code < KEY_CODES_SIZE){
        debug("Key code found ");debug_c(KEY_CODES[scan_code]);
        debug(", ");debug_i(scan_code,16);debug("\n");
        return KEY_CODES[scan_code];
    }
    debug("Unknown key code ");debug_i(scan_code,16);debug("\n");
    return 0;
}

static void read_keyboard(){
    uint8_t state = 0;
    uint8_t val;
    uint16_t key_code;

    val = ps2_read(0);
    if (val == SCAN_ALTCODE){
        state |= FLAG_ALTCODE;
        val = ps2_read(0);
    }
    if (val & 0x80){
        debug("Release\n");
        state |= FLAG_BREAK;
        val &= 0x7F;
    }
    key_code = scan_code_to_key_code(state & FLAG_ALTCODE, val);
    if (key_code){
        KEYBOARD.key_event.press_release = (state & FLAG_ALTCODE) != 0;
        KEYBOARD.key_event.keycode = key_code;
    }
}

static void handle_keyboard_irq (InterruptFrame* frame, void* data){
   cli();
   debug("Keyboard input\n");

   read_keyboard();

   release_lock(&(KEYBOARD.lock));
   pic_eoi1();
   sti();
}


