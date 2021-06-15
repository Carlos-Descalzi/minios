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

#define BUFFER_SIZE 256

static uint8_t  count_devices       (DeviceType* device_type);
static Device*  instantiate         (DeviceType* device_type, uint8_t device_number);
static void     release             (DeviceType* device_type, Device* device);
static int16_t  setopt              (Device* device, uint32_t option, void* data);
static int16_t  read                (CharDevice* device);
static void     handle_keyboard_irq (InterruptFrame* frame);

typedef struct {
    CharDevice device;
    uint8_t* buffer;
    uint8_t buffptr;
    uint8_t readptr;
    uint8_t mode;
    uint32_t ticks;
    Lock lock;
} KeyboardDevice;

#define KEYBOARD_DEVICE(d)  ((KeyboardDevice*)d)

#define OPT_MODE    0x01

#define OPT_MODE_RAW    0x00
#define OPT_MODE_TERM   0x01

static DeviceType DEVICE_TYPE = {
    kind: KBD,
    count_devices: count_devices,
    instantiate: instantiate,
    release: release
};

static KeyboardDevice KBD_DEVICE = {
    device: {
        base: {
            type: DEVICE_TYPE_CHAR,
            setopt: setopt,
        },
        read: read,
        write: NULL
    },
    buffer: NULL,
    mode: OPT_MODE_RAW,
    ticks: 0,
    lock: 0
};
static void set_mode(uint32_t mode);

void keyboard_register(){
    device_register_type((DeviceType*)&DEVICE_TYPE);
}

static uint8_t count_devices(DeviceType* device_type){
    return 1;
}
static Device* instantiate(DeviceType* device_type, uint8_t device_number){
    KBD_DEVICE.buffer = heap_alloc(BUFFER_SIZE);
    KBD_DEVICE.buffptr = 0;
    debug("Initiating keyboard driver\n");

    isr_install(PIC_IRQ_BASE + PS2_IRQ, handle_keyboard_irq);
    return DEVICE(&KBD_DEVICE);
}
static void release(DeviceType* device_type, Device* device){
    heap_free(KBD_DEVICE.buffer);
    isr_install(PIC_IRQ_BASE + PS2_IRQ, NULL);
    acquire_lock(&(KBD_DEVICE.lock));
}
static int16_t setopt(Device* device, uint32_t option, void* data){
    if (option == OPT_MODE){
        set_mode((uint32_t)data);
    }
    return 0;
}

static int16_t read(CharDevice* device){
    int16_t val;
    while (KBD_DEVICE.readptr == KBD_DEVICE.buffptr){
        acquire_lock(&(KBD_DEVICE.lock));
    }
    val = KBD_DEVICE.buffer[KBD_DEVICE.readptr++];
    KBD_DEVICE.readptr++;

    if (KBD_DEVICE.readptr >= BUFFER_SIZE){
        KBD_DEVICE.readptr = 0;
    }
    return val;
}

static void handle_keyboard_irq (InterruptFrame* frame){
   uint8_t val;
   cli();
   debug("Keyboard input\n");

   val = ps2_read(0);

   if (KBD_DEVICE.buffptr != KBD_DEVICE.readptr){
       KBD_DEVICE.buffer[KBD_DEVICE.buffptr++] = val;

       if (KBD_DEVICE.buffptr >= BUFFER_SIZE){
           KBD_DEVICE.buffptr = 0;
       }
   }
   release_lock(&(KBD_DEVICE.lock));
   pic_eoi1();
   sti();
}
static void raw_read(CharDevice* device);

static void set_mode(uint32_t mode){
}

static void raw_read(CharDevice* device){
}

static void raw_handle_irq(InterruptFrame* frame){
}

static void term_read(CharDevice* device){
}

static void term_handle_irq(InterruptFrame* frame){
}
